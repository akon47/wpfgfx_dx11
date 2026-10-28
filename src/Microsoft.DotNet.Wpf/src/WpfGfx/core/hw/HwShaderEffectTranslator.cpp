// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      D3D9 픽셀 셰이더 바이트코드(ps_2_0 ~ ps_3_0) → HLSL 변환기. HwShaderEffectTranslator.h 참고.
//
//      토큰 형식은 d3d9types.h 의 D3DSI_*/D3DSP_* 매크로를 그대로 쓴다. 명령 하나가 HLSL
//      한 줄이 되고, 레지스터는 지역 float4 변수(r#, v#, t#, oC0)와 cbuffer 배열(c[], i[], b[])로
//      대응한다. def/defi/defb 로 박힌 리터럴은 지역 상수가 된다(앱이 그 레지스터를 덮어쓰지
//      않는다고 가정한다. D3D9 에서도 리터럴은 컴파일러가 빈 레지스터에 둔다).
//
//------------------------------------------------------------------------------

#include "precomp.hpp"
#include "d3d9interop.h"
#include <string>

namespace
{

//
// 레지스터 종류 (D3DSHADER_PARAM_REGISTER_TYPE 와 같은 값)
//
enum RegType
{
    Reg_Temp = 0,
    Reg_Input = 1,
    Reg_Const = 2,
    Reg_Texture = 3,      // ps_2_0 텍스처 좌표 입력 t#
    Reg_ConstInt = 7,
    Reg_ColorOut = 8,
    Reg_DepthOut = 9,
    Reg_Sampler = 10,
    Reg_ConstBool = 14,
    Reg_Loop = 15,
    Reg_Misc = 17,
    Reg_Label = 18,
    Reg_Predicate = 19,
};

const UINT c_cTemps = 32;
const UINT c_cInputs = 10;     // ps_3_0 v0~v9
const UINT c_cTexCoords = 8;   // ps_2_0 t0~t7
const UINT c_cSamplers = 16;
const UINT c_cFloatConsts = 224;
const UINT c_cIntConsts = 16;
const UINT c_cBoolConsts = 16;

class CTranslator
{
public:
    CTranslator(const DWORD *pTokens, SIZE_T cTokens)
        : m_p(pTokens), m_pEnd(pTokens + cTokens), m_uMajor(0), m_uMinor(0),
          m_cLoopDepth(0), m_cBlockDepth(0), m_fOutputWritten(false)
    {
        ZeroMemory(m_rgfSampler, sizeof(m_rgfSampler));
        ZeroMemory(m_rgfConstDef, sizeof(m_rgfConstDef));
        ZeroMemory(m_rgfIntDef, sizeof(m_rgfIntDef));
        ZeroMemory(m_rgfBoolDef, sizeof(m_rgfBoolDef));
        ZeroMemory(m_rgInputSource, sizeof(m_rgInputSource));
        ZeroMemory(m_rgfInputDeclared, sizeof(m_rgfInputDeclared));
    }

    HRESULT Translate(__out std::string *pHLSL);

private:
    struct Operand
    {
        DWORD tok;
        UINT type;
        UINT num;
        bool fRelative;
        DWORD relTok;
    };

    HRESULT ReadOperand(__out Operand *pOp);
    HRESULT RegisterName(const Operand &op, __out std::string *pName);
    HRESULT SourceText(const Operand &op, __out std::string *pText);
    HRESULT DestAssign(const Operand &op, const std::string &expr, __out std::string *pLine);
    HRESULT SamplerIndex(const Operand &op, __out UINT *pIndex);
    HRESULT Comparison(DWORD instTok, __out const char **ppsz);
    HRESULT MatrixOp(const Operand &dst, const Operand &s0, const Operand &s1, UINT rows, UINT cols);
    void Line(const std::string &s);
    HRESULT Fail(const char *pszWhat);

    const DWORD *m_p;
    const DWORD *m_pEnd;
    UINT m_uMajor;
    UINT m_uMinor;

    std::string m_body;
    std::string m_defs;
    bool m_rgfSampler[c_cSamplers];
    bool m_rgfConstDef[c_cFloatConsts];
    bool m_rgfIntDef[c_cIntConsts];
    bool m_rgfBoolDef[c_cBoolConsts];

    // ps_3_0 입력 선언: 0 = 없음, 1 = TEXCOORD0, 2 = COLOR0, 3 = 그 밖(0 으로 채움)
    BYTE m_rgInputSource[c_cInputs];
    bool m_rgfInputDeclared[c_cInputs];

    UINT m_cLoopDepth;
    UINT m_cBlockDepth;
    bool m_fOutputWritten;
};

static std::string Fmt(__in_z const char *pszFormat, ...)
{
    char sz[512];
    va_list args;
    va_start(args, pszFormat);
    _vsnprintf_s(sz, ARRAYSIZE(sz), _TRUNCATE, pszFormat, args);
    va_end(args);
    return std::string(sz);
}

static std::string FloatLiteral(DWORD dw)
{
    float f;
    CopyMemory(&f, &dw, sizeof(f));

    if (_isnan(f) || !_finite(f))
    {
        return "0.0";
    }

    return Fmt("%.9g", f);
}

void CTranslator::Line(const std::string &s)
{
    for (UINT i = 0; i < m_cBlockDepth + 1; i++)
    {
        m_body += "    ";
    }

    m_body += s;
    m_body += "\n";
}

HRESULT CTranslator::Fail(const char *pszWhat)
{
    MIL_DX11_LOG("ShaderEffect translator: unsupported %s (ps_%u_%u)", pszWhat, m_uMajor, m_uMinor);
    return WGXERR_SHADER_COMPILE_FAILED;
}

HRESULT CTranslator::ReadOperand(__out Operand *pOp)
{
    HRESULT hr = S_OK;

    if (m_p >= m_pEnd)
    {
        IFC(WGXERR_SHADER_COMPILE_FAILED);
    }

    pOp->tok = *m_p++;
    pOp->type = ((pOp->tok & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT)
              | ((pOp->tok & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2);
    pOp->num = pOp->tok & D3DSP_REGNUM_MASK;
    pOp->fRelative = (pOp->tok & D3DSHADER_ADDRESSMODE_MASK) != 0;
    pOp->relTok = 0;

    if (pOp->fRelative)
    {
        if (m_p >= m_pEnd)
        {
            IFC(WGXERR_SHADER_COMPILE_FAILED);
        }

        pOp->relTok = *m_p++;
    }

Cleanup:
    RRETURN(hr);
}

HRESULT CTranslator::RegisterName(const Operand &op, __out std::string *pName)
{
    HRESULT hr = S_OK;

    switch (op.type)
    {
    case Reg_Temp:
        if (op.num >= c_cTemps) IFC(Fail("temp register number"));
        *pName = Fmt("r%u", op.num);
        break;

    case Reg_Input:
        if (op.num >= c_cInputs) IFC(Fail("input register number"));
        *pName = Fmt("v%u", op.num);
        break;

    case Reg_Texture:
        if (op.num >= c_cTexCoords) IFC(Fail("texture coordinate register number"));
        *pName = Fmt("t%u", op.num);
        break;

    case Reg_Const:
        if (op.num >= c_cFloatConsts) IFC(Fail("constant register number"));

        if (op.fRelative)
        {
            UINT relType = ((op.relTok & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT)
                         | ((op.relTok & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2);

            if (relType != Reg_Loop) IFC(Fail("relative addressing register"));

            *pName = Fmt("c[aL + %u]", op.num);
        }
        else if (m_rgfConstDef[op.num])
        {
            *pName = Fmt("cdef%u", op.num);
        }
        else
        {
            *pName = Fmt("c[%u]", op.num);
        }
        break;

    case Reg_ConstInt:
        if (op.num >= c_cIntConsts) IFC(Fail("integer constant number"));
        *pName = m_rgfIntDef[op.num] ? Fmt("idef%u", op.num) : Fmt("i[%u]", op.num);
        break;

    case Reg_ConstBool:
        if (op.num >= c_cBoolConsts) IFC(Fail("boolean constant number"));
        *pName = m_rgfBoolDef[op.num] ? Fmt("bdef%u", op.num) : Fmt("b[%u]", op.num);
        break;

    case Reg_ColorOut:
        if (op.num != 0) IFC(Fail("multiple render targets"));
        *pName = "oC0";
        break;

    case Reg_Loop:
        *pName = "aLvec";
        break;

    case Reg_Misc:
        if (op.num == D3DSMO_POSITION)
        {
            *pName = "vPos";
        }
        else
        {
            IFC(Fail("vFace register"));
        }
        break;

    case Reg_DepthOut:
        IFC(Fail("depth output"));
        break;

    case Reg_Predicate:
        IFC(Fail("predicate register"));
        break;

    default:
        IFC(Fail("register type"));
        break;
    }

Cleanup:
    RRETURN(hr);
}

HRESULT CTranslator::SourceText(const Operand &op, __out std::string *pText)
{
    HRESULT hr = S_OK;
    std::string name;

    IFC(RegisterName(op, &name));

    //
    // Swizzle: 2 bits per component, x = 0 .. w = 3
    //
    {
        static const char rgch[] = "xyzw";
        DWORD sw = (op.tok & D3DSP_SWIZZLE_MASK) >> D3DSP_SWIZZLE_SHIFT;
        char szSwizzle[6] = ".xyzw";

        for (UINT k = 0; k < 4; k++)
        {
            szSwizzle[1 + k] = rgch[(sw >> (2 * k)) & 3];
        }

        name += szSwizzle;
    }

    switch (op.tok & D3DSP_SRCMOD_MASK)
    {
    case D3DSPSM_NONE:
        *pText = name;
        break;

    case D3DSPSM_NEG:
        *pText = "(-" + name + ")";
        break;

    case D3DSPSM_ABS:
        *pText = "abs(" + name + ")";
        break;

    case D3DSPSM_ABSNEG:
        *pText = "(-abs(" + name + "))";
        break;

    case D3DSPSM_BIAS:
        *pText = "(" + name + " - 0.5)";
        break;

    case D3DSPSM_BIASNEG:
        *pText = "(0.5 - " + name + ")";
        break;

    case D3DSPSM_SIGN:
        *pText = "(2.0 * (" + name + " - 0.5))";
        break;

    case D3DSPSM_SIGNNEG:
        *pText = "(-2.0 * (" + name + " - 0.5))";
        break;

    case D3DSPSM_COMP:
        *pText = "(1.0 - " + name + ")";
        break;

    case D3DSPSM_X2:
        *pText = "(2.0 * " + name + ")";
        break;

    case D3DSPSM_X2NEG:
        *pText = "(-2.0 * " + name + ")";
        break;

    default:
        IFC(Fail("source modifier"));
        break;
    }

    //
    // Integer and boolean registers are int4 in the constant buffer.
    //
    if (op.type == Reg_ConstInt || op.type == Reg_ConstBool || op.type == Reg_Loop)
    {
        *pText = "((float4)" + *pText + ")";
    }

Cleanup:
    RRETURN(hr);
}

HRESULT CTranslator::DestAssign(const Operand &op, const std::string &expr, __out std::string *pLine)
{
    HRESULT hr = S_OK;
    std::string name;

    if (op.type == Reg_ColorOut)
    {
        m_fOutputWritten = true;
    }
    else if (op.type != Reg_Temp)
    {
        IFC(Fail("destination register type"));
    }

    IFC(RegisterName(op, &name));

    {
        DWORD mask = (op.tok & D3DSP_WRITEMASK_ALL) >> 16;
        char szMask[6] = ".";
        UINT c = 1;

        if (mask & 1) szMask[c++] = 'x';
        if (mask & 2) szMask[c++] = 'y';
        if (mask & 4) szMask[c++] = 'z';
        if (mask & 8) szMask[c++] = 'w';
        szMask[c] = '\0';

        if (mask == 0 || mask == 0xF)
        {
            szMask[0] = '\0';
        }

        std::string value = expr;

        if (op.tok & D3DSPDM_SATURATE)
        {
            value = "saturate(" + value + ")";
        }

        if (szMask[0] != '\0')
        {
            *pLine = name + szMask + " = (" + value + ")" + szMask + ";";
        }
        else
        {
            *pLine = name + " = " + value + ";";
        }
    }

Cleanup:
    RRETURN(hr);
}

HRESULT CTranslator::SamplerIndex(const Operand &op, __out UINT *pIndex)
{
    HRESULT hr = S_OK;

    if (op.type != Reg_Sampler || op.num >= c_cSamplers)
    {
        IFC(Fail("sampler operand"));
    }

    if (!m_rgfSampler[op.num])
    {
        // Samplers without a declaration are 2D textures (ps_2_0 asm shaders)
        m_rgfSampler[op.num] = true;
    }

    *pIndex = op.num;

Cleanup:
    RRETURN(hr);
}

HRESULT CTranslator::Comparison(DWORD instTok, __out const char **ppsz)
{
    HRESULT hr = S_OK;

    switch ((instTok & D3DSHADER_COMPARISON_MASK) >> D3DSHADER_COMPARISON_SHIFT)
    {
    case D3DSPC_GT: *ppsz = ">";  break;
    case D3DSPC_EQ: *ppsz = "=="; break;
    case D3DSPC_GE: *ppsz = ">="; break;
    case D3DSPC_LT: *ppsz = "<";  break;
    case D3DSPC_NE: *ppsz = "!="; break;
    case D3DSPC_LE: *ppsz = "<="; break;
    default:
        IFC(Fail("comparison"));
        break;
    }

Cleanup:
    RRETURN(hr);
}

//
// m4x4 등: 결과의 행마다 src1 의 연속 레지스터와 내적한다.
//
HRESULT CTranslator::MatrixOp(const Operand &dst, const Operand &s0, const Operand &s1, UINT rows, UINT cols)
{
    HRESULT hr = S_OK;
    std::string a, expr = "float4(";

    IFC(SourceText(s0, &a));

    for (UINT r = 0; r < 4; r++)
    {
        if (r > 0) expr += ", ";

        if (r < rows)
        {
            Operand row = s1;
            row.num += r;
            row.tok = (row.tok & ~D3DSP_REGNUM_MASK) | (row.num & D3DSP_REGNUM_MASK);

            std::string b;
            IFC(SourceText(row, &b));

            expr += (cols == 4) ? ("dot(" + a + ", " + b + ")") : ("dot((" + a + ").xyz, (" + b + ").xyz)");
        }
        else
        {
            expr += "0.0";
        }
    }

    expr += ")";

    {
        std::string line;
        IFC(DestAssign(dst, expr, &line));
        Line(line);
    }

Cleanup:
    RRETURN(hr);
}

HRESULT CTranslator::Translate(__out std::string *pHLSL)
{
    HRESULT hr = S_OK;

    //
    // Version token
    //
    if (m_p >= m_pEnd)
    {
        IFC(WGXERR_SHADER_COMPILE_FAILED);
    }

    {
        DWORD version = *m_p++;

        if ((version >> 16) != 0xFFFF)
        {
            IFC(Fail("shader type (not a pixel shader)"));
        }

        m_uMajor = (version >> 8) & 0xFF;
        m_uMinor = version & 0xFF;

        if (m_uMajor < 2 || m_uMajor > 3)
        {
            IFC(Fail("shader version"));
        }
    }

    while (m_p < m_pEnd)
    {
        DWORD inst = *m_p++;
        DWORD opcode = inst & D3DSI_OPCODE_MASK;

        if (opcode == D3DSIO_END)
        {
            break;
        }

        if (opcode == D3DSIO_COMMENT)
        {
            m_p += (inst & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;
            continue;
        }

        if (inst & D3DSHADER_INSTRUCTION_PREDICATED)
        {
            IFC(Fail("predicated instruction"));
        }

        UINT cParams = (inst & D3DSI_INSTLENGTH_MASK) >> D3DSI_INSTLENGTH_SHIFT;
        const DWORD *pNext = m_p + cParams;

        if (pNext > m_pEnd)
        {
            IFC(WGXERR_SHADER_COMPILE_FAILED);
        }

        Operand dst, s0, s1, s2, s3;
        std::string a, b, c, line;

        switch (opcode)
        {
        case D3DSIO_NOP:
            break;

        case D3DSIO_DCL:
        {
            DWORD dcl = *m_p++;
            Operand reg;
            IFC(ReadOperand(&reg));

            if (reg.type == Reg_Sampler)
            {
                DWORD texType = dcl & D3DSP_TEXTURETYPE_MASK;

                if (texType != D3DSTT_2D && texType != D3DSTT_UNKNOWN)
                {
                    IFC(Fail("non-2D sampler"));
                }

                if (reg.num >= c_cSamplers) IFC(Fail("sampler number"));
                m_rgfSampler[reg.num] = true;
            }
            else if (reg.type == Reg_Input && m_uMajor >= 3)
            {
                if (reg.num >= c_cInputs) IFC(Fail("input register number"));

                UINT usage = dcl & D3DSP_DCL_USAGE_MASK;
                UINT usageIndex = (dcl & D3DSP_DCL_USAGEINDEX_MASK) >> D3DSP_DCL_USAGEINDEX_SHIFT;

                m_rgfInputDeclared[reg.num] = true;

                if (usage == D3DDECLUSAGE_TEXCOORD && usageIndex == 0)
                {
                    m_rgInputSource[reg.num] = 1;
                }
                else if (usage == D3DDECLUSAGE_COLOR && usageIndex == 0)
                {
                    m_rgInputSource[reg.num] = 2;
                }
                else
                {
                    m_rgInputSource[reg.num] = 3;
                }
            }
            else if (reg.type == Reg_Input || reg.type == Reg_Texture || reg.type == Reg_Misc)
            {
                // ps_2_0 dcl t#/v#: fixed meaning, nothing to record
            }
            else
            {
                IFC(Fail("declaration"));
            }
            break;
        }

        case D3DSIO_DEF:
        {
            IFC(ReadOperand(&dst));
            if (dst.type != Reg_Const || dst.num >= c_cFloatConsts) IFC(Fail("def register"));
            if (m_p + 4 > m_pEnd) IFC(WGXERR_SHADER_COMPILE_FAILED);

            m_rgfConstDef[dst.num] = true;
            m_defs += Fmt("    const float4 cdef%u = float4(", dst.num)
                    + FloatLiteral(m_p[0]) + ", " + FloatLiteral(m_p[1]) + ", "
                    + FloatLiteral(m_p[2]) + ", " + FloatLiteral(m_p[3]) + ");\n";
            m_p += 4;
            break;
        }

        case D3DSIO_DEFI:
        {
            IFC(ReadOperand(&dst));
            if (dst.type != Reg_ConstInt || dst.num >= c_cIntConsts) IFC(Fail("defi register"));
            if (m_p + 4 > m_pEnd) IFC(WGXERR_SHADER_COMPILE_FAILED);

            m_rgfIntDef[dst.num] = true;
            m_defs += Fmt("    const int4 idef%u = int4(%d, %d, %d, %d);\n", dst.num,
                          static_cast<int>(m_p[0]), static_cast<int>(m_p[1]),
                          static_cast<int>(m_p[2]), static_cast<int>(m_p[3]));
            m_p += 4;
            break;
        }

        case D3DSIO_DEFB:
        {
            IFC(ReadOperand(&dst));
            if (dst.type != Reg_ConstBool || dst.num >= c_cBoolConsts) IFC(Fail("defb register"));
            if (m_p + 1 > m_pEnd) IFC(WGXERR_SHADER_COMPILE_FAILED);

            m_rgfBoolDef[dst.num] = true;
            m_defs += Fmt("    const int4 bdef%u = int4(%d, %d, %d, %d);\n", dst.num,
                          m_p[0] ? 1 : 0, m_p[0] ? 1 : 0, m_p[0] ? 1 : 0, m_p[0] ? 1 : 0);
            m_p += 1;
            break;
        }

        //
        // Arithmetic
        //
        case D3DSIO_MOV:
        case D3DSIO_ABS:
        case D3DSIO_FRC:
        case D3DSIO_DSX:
        case D3DSIO_DSY:
        case D3DSIO_NRM:
        case D3DSIO_RCP:
        case D3DSIO_RSQ:
        case D3DSIO_EXP:
        case D3DSIO_EXPP:
        case D3DSIO_LOG:
        case D3DSIO_LOGP:
        case D3DSIO_LIT:
        {
            IFC(ReadOperand(&dst));
            IFC(ReadOperand(&s0));
            IFC(SourceText(s0, &a));

            std::string expr;

            switch (opcode)
            {
            case D3DSIO_MOV:  expr = a; break;
            case D3DSIO_ABS:  expr = "abs(" + a + ")"; break;
            case D3DSIO_FRC:  expr = "frac(" + a + ")"; break;
            case D3DSIO_DSX:  expr = "ddx(" + a + ")"; break;
            case D3DSIO_DSY:  expr = "ddy(" + a + ")"; break;
            case D3DSIO_NRM:  expr = "(" + a + " * rsqrt(dot((" + a + ").xyz, (" + a + ").xyz)))"; break;
            case D3DSIO_RCP:  expr = "((float4)(1.0 / (" + a + ").x))"; break;
            case D3DSIO_RSQ:  expr = "((float4)rsqrt(abs((" + a + ").x)))"; break;
            case D3DSIO_EXP:
            case D3DSIO_EXPP: expr = "((float4)exp2((" + a + ").x))"; break;
            case D3DSIO_LOG:
            case D3DSIO_LOGP: expr = "((float4)log2(abs((" + a + ").x)))"; break;
            case D3DSIO_LIT:  expr = "lit((" + a + ").x, (" + a + ").y, (" + a + ").w)"; break;
            }

            IFC(DestAssign(dst, expr, &line));
            Line(line);
            break;
        }

        case D3DSIO_ADD:
        case D3DSIO_SUB:
        case D3DSIO_MUL:
        case D3DSIO_MIN:
        case D3DSIO_MAX:
        case D3DSIO_SLT:
        case D3DSIO_SGE:
        case D3DSIO_DP3:
        case D3DSIO_DP4:
        case D3DSIO_POW:
        case D3DSIO_CRS:
        case D3DSIO_DST:
        {
            IFC(ReadOperand(&dst));
            IFC(ReadOperand(&s0));
            IFC(ReadOperand(&s1));
            IFC(SourceText(s0, &a));
            IFC(SourceText(s1, &b));

            std::string expr;

            switch (opcode)
            {
            case D3DSIO_ADD: expr = "(" + a + " + " + b + ")"; break;
            case D3DSIO_SUB: expr = "(" + a + " - " + b + ")"; break;
            case D3DSIO_MUL: expr = "(" + a + " * " + b + ")"; break;
            case D3DSIO_MIN: expr = "min(" + a + ", " + b + ")"; break;
            case D3DSIO_MAX: expr = "max(" + a + ", " + b + ")"; break;
            case D3DSIO_SLT: expr = "((" + a + " < " + b + ") ? float4(1,1,1,1) : float4(0,0,0,0))"; break;
            case D3DSIO_SGE: expr = "((" + a + " >= " + b + ") ? float4(1,1,1,1) : float4(0,0,0,0))"; break;
            case D3DSIO_DP3: expr = "((float4)dot((" + a + ").xyz, (" + b + ").xyz))"; break;
            case D3DSIO_DP4: expr = "((float4)dot(" + a + ", " + b + "))"; break;
            case D3DSIO_POW: expr = "((float4)pow(abs((" + a + ").x), (" + b + ").x))"; break;
            case D3DSIO_CRS: expr = "float4(cross((" + a + ").xyz, (" + b + ").xyz), 0.0)"; break;
            case D3DSIO_DST: expr = "dst(" + a + ", " + b + ")"; break;
            }

            IFC(DestAssign(dst, expr, &line));
            Line(line);
            break;
        }

        case D3DSIO_MAD:
        case D3DSIO_LRP:
        case D3DSIO_CMP:
        case D3DSIO_CND:
        case D3DSIO_DP2ADD:
        {
            IFC(ReadOperand(&dst));
            IFC(ReadOperand(&s0));
            IFC(ReadOperand(&s1));
            IFC(ReadOperand(&s2));
            IFC(SourceText(s0, &a));
            IFC(SourceText(s1, &b));
            IFC(SourceText(s2, &c));

            std::string expr;

            switch (opcode)
            {
            case D3DSIO_MAD:    expr = "(" + a + " * " + b + " + " + c + ")"; break;
            case D3DSIO_LRP:    expr = "lerp(" + c + ", " + b + ", " + a + ")"; break;
            case D3DSIO_CMP:    expr = "((" + a + " >= float4(0,0,0,0)) ? " + b + " : " + c + ")"; break;
            case D3DSIO_CND:    expr = "((" + a + " > float4(0.5,0.5,0.5,0.5)) ? " + b + " : " + c + ")"; break;
            case D3DSIO_DP2ADD: expr = "((float4)(dot((" + a + ").xy, (" + b + ").xy) + (" + c + ").x))"; break;
            }

            IFC(DestAssign(dst, expr, &line));
            Line(line);
            break;
        }

        case D3DSIO_SINCOS:
        {
            IFC(ReadOperand(&dst));
            IFC(ReadOperand(&s0));
            IFC(SourceText(s0, &a));
            // ps_2_0 carries two constant operands that only the macro expansion used
            IFC(DestAssign(dst, "float4(cos((" + a + ").x), sin((" + a + ").x), 0.0, 0.0)", &line));
            Line(line);
            break;
        }

        case D3DSIO_M4x4:
        case D3DSIO_M4x3:
        case D3DSIO_M3x4:
        case D3DSIO_M3x3:
        case D3DSIO_M3x2:
        {
            IFC(ReadOperand(&dst));
            IFC(ReadOperand(&s0));
            IFC(ReadOperand(&s1));

            UINT rows = (opcode == D3DSIO_M4x4 || opcode == D3DSIO_M3x4) ? 4
                      : (opcode == D3DSIO_M4x3 || opcode == D3DSIO_M3x3) ? 3 : 2;
            UINT cols = (opcode == D3DSIO_M4x4 || opcode == D3DSIO_M4x3) ? 4 : 3;

            IFC(MatrixOp(dst, s0, s1, rows, cols));
            break;
        }

        //
        // Texturing
        //
        case D3DSIO_TEX:
        case D3DSIO_TEXLDL:
        case D3DSIO_TEXLDD:
        {
            UINT uSampler;

            IFC(ReadOperand(&dst));
            IFC(ReadOperand(&s0));
            IFC(ReadOperand(&s1));
            IFC(SourceText(s0, &a));
            IFC(SamplerIndex(s1, &uSampler));

            std::string tex = Fmt("tex%u", uSampler);
            std::string samp = Fmt("samp%u", uSampler);
            std::string expr;

            if (opcode == D3DSIO_TEXLDD)
            {
                std::string d, e;
                IFC(ReadOperand(&s2));
                IFC(ReadOperand(&s3));
                IFC(SourceText(s2, &d));
                IFC(SourceText(s3, &e));
                expr = tex + ".SampleGrad(" + samp + ", (" + a + ").xy, (" + d + ").xy, (" + e + ").xy)";
            }
            else if (opcode == D3DSIO_TEXLDL)
            {
                expr = tex + ".SampleLevel(" + samp + ", (" + a + ").xy, (" + a + ").w)";
            }
            else if (inst & D3DSI_TEXLD_PROJECT)
            {
                expr = tex + ".Sample(" + samp + ", (" + a + ").xy / (" + a + ").w)";
            }
            else if (inst & D3DSI_TEXLD_BIAS)
            {
                expr = tex + ".SampleBias(" + samp + ", (" + a + ").xy, (" + a + ").w)";
            }
            else
            {
                expr = tex + ".Sample(" + samp + ", (" + a + ").xy)";
            }

            IFC(DestAssign(dst, expr, &line));
            Line(line);
            break;
        }

        case D3DSIO_TEXKILL:
        {
            IFC(ReadOperand(&dst));   // encoded as a destination token

            Operand src = dst;
            src.tok = (src.tok & ~(D3DSP_SWIZZLE_MASK | D3DSP_SRCMOD_MASK)) | D3DSP_NOSWIZZLE;

            IFC(SourceText(src, &a));
            Line(std::string("clip((") + a + ((m_uMajor >= 3) ? ").xyzw);" : ").xyz);"));
            break;
        }

        //
        // Flow control
        //
        case D3DSIO_IF:
        {
            IFC(ReadOperand(&s0));
            if (s0.type != Reg_ConstBool) IFC(Fail("if on a predicate"));
            IFC(SourceText(s0, &a));
            Line("if ((" + a + ").x != 0.0) {");
            m_cBlockDepth++;
            break;
        }

        case D3DSIO_IFC:
        {
            const char *pszCmp;
            IFC(Comparison(inst, &pszCmp));
            IFC(ReadOperand(&s0));
            IFC(ReadOperand(&s1));
            IFC(SourceText(s0, &a));
            IFC(SourceText(s1, &b));
            Line("if ((" + a + ").x " + pszCmp + " (" + b + ").x) {");
            m_cBlockDepth++;
            break;
        }

        case D3DSIO_ELSE:
            if (m_cBlockDepth == 0) IFC(WGXERR_SHADER_COMPILE_FAILED);
            m_cBlockDepth--;
            Line("} else {");
            m_cBlockDepth++;
            break;

        case D3DSIO_ENDIF:
            if (m_cBlockDepth == 0) IFC(WGXERR_SHADER_COMPILE_FAILED);
            m_cBlockDepth--;
            Line("}");
            break;

        case D3DSIO_LOOP:
        {
            IFC(ReadOperand(&s0));   // aL
            IFC(ReadOperand(&s1));   // i#
            if (s0.type != Reg_Loop || s1.type != Reg_ConstInt) IFC(Fail("loop operands"));

            std::string ctr;
            IFC(RegisterName(s1, &ctr));

            Line(Fmt("[loop] for (int _k%u = 0, aL = %s.y; _k%u < %s.x; _k%u++, aL += %s.z) {",
                     m_cLoopDepth, ctr.c_str(), m_cLoopDepth, ctr.c_str(), m_cLoopDepth, ctr.c_str()));
            Line("int4 aLvec = int4(aL, aL, aL, aL);");
            m_cLoopDepth++;
            m_cBlockDepth++;
            break;
        }

        case D3DSIO_REP:
        {
            IFC(ReadOperand(&s0));
            if (s0.type != Reg_ConstInt) IFC(Fail("rep operand"));

            std::string ctr;
            IFC(RegisterName(s0, &ctr));

            Line(Fmt("[loop] for (int _k%u = 0; _k%u < %s.x; _k%u++) {", m_cLoopDepth, m_cLoopDepth, ctr.c_str(), m_cLoopDepth));
            m_cLoopDepth++;
            m_cBlockDepth++;
            break;
        }

        case D3DSIO_ENDLOOP:
        case D3DSIO_ENDREP:
            if (m_cBlockDepth == 0 || m_cLoopDepth == 0) IFC(WGXERR_SHADER_COMPILE_FAILED);
            m_cBlockDepth--;
            m_cLoopDepth--;
            Line("}");
            break;

        case D3DSIO_BREAK:
            Line("break;");
            break;

        case D3DSIO_BREAKC:
        {
            const char *pszCmp;
            IFC(Comparison(inst, &pszCmp));
            IFC(ReadOperand(&s0));
            IFC(ReadOperand(&s1));
            IFC(SourceText(s0, &a));
            IFC(SourceText(s1, &b));
            Line("if ((" + a + ").x " + pszCmp + " (" + b + ").x) break;");
            break;
        }

        default:
            IFC(Fail(Fmt("opcode %u", opcode).c_str()));
            break;
        }

        // Skip anything the opcode-specific reader did not consume (ps_2_0 sincos constants)
        if (m_p < pNext)
        {
            m_p = pNext;
        }
        else if (m_p > pNext)
        {
            IFC(WGXERR_SHADER_COMPILE_FAILED);
        }
    }

    if (m_cBlockDepth != 0)
    {
        IFC(WGXERR_SHADER_COMPILE_FAILED);
    }

    //
    // Assemble the HLSL
    //
    {
        std::string hlsl;

        hlsl += Fmt("// Translated from D3D9 ps_%u_%u bytecode\n", m_uMajor, m_uMinor);
        hlsl += Fmt("cbuffer MilPixelShaderConstants : register(b0) { float4 c[%u]; };\n", c_cFloatConsts);
        hlsl += Fmt("cbuffer MilPixelShaderIntBoolConstants : register(b1) { int4 i[%u]; int4 b[%u]; };\n", c_cIntConsts, c_cBoolConsts);

        for (UINT s = 0; s < c_cSamplers; s++)
        {
            if (m_rgfSampler[s])
            {
                hlsl += Fmt("Texture2D tex%u : register(t%u);\nSamplerState samp%u : register(s%u);\n", s, s, s, s);
            }
        }

        hlsl += "struct PSIn { float4 Pos : SV_POSITION; float4 Color : COLOR0; float2 UV : TEXCOORD0; };\n";
        hlsl += "float4 main(PSIn In) : SV_TARGET\n{\n";
        hlsl += m_defs;

        for (UINT r = 0; r < c_cTemps; r++)
        {
            hlsl += Fmt("    float4 r%u = float4(0,0,0,0);\n", r);
        }

        if (m_uMajor >= 3)
        {
            for (UINT v = 0; v < c_cInputs; v++)
            {
                const char *pszValue = "float4(0,0,0,0)";

                switch (m_rgInputSource[v])
                {
                case 1: pszValue = "float4(In.UV, 0.0, 1.0)"; break;
                case 2: pszValue = "In.Color"; break;
                default: break;
                }

                hlsl += Fmt("    float4 v%u = %s;\n", v, pszValue);
            }
        }
        else
        {
            hlsl += "    float4 v0 = In.Color;\n    float4 v1 = float4(0,0,0,0);\n";

            for (UINT t = 0; t < c_cTexCoords; t++)
            {
                hlsl += Fmt("    float4 t%u = %s;\n", t, (t == 0) ? "float4(In.UV, 0.0, 1.0)" : "float4(0,0,0,0)");
            }
        }

        hlsl += "    float4 vPos = float4(In.Pos.xy - 0.5, In.Pos.zw);\n";
        hlsl += "    float4 oC0 = float4(0,0,0,0);\n";
        hlsl += m_body;
        hlsl += "    return oC0;\n}\n";

        *pHLSL = hlsl;
    }

Cleanup:
    RRETURN(hr);
}

} // namespace

HRESULT
MilTranslateD3D9PixelShaderToHLSL(
    __in_bcount(cbBytecode) const void *pvBytecode,
    SIZE_T cbBytecode,
    __out std::string *pHLSL
    )
{
    HRESULT hr = S_OK;

    if (pvBytecode == NULL || cbBytecode < sizeof(DWORD) * 2 || (cbBytecode % sizeof(DWORD)) != 0)
    {
        IFC(WGXERR_SHADER_COMPILE_FAILED);
    }

    {
        CTranslator translator(static_cast<const DWORD *>(pvBytecode), cbBytecode / sizeof(DWORD));
        IFC(translator.Translate(pHLSL));
    }

Cleanup:
    RRETURN(hr);
}
