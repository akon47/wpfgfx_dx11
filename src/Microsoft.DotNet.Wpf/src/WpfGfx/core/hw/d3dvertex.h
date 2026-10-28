// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  $TAG ENGR
//
//      $Module:    win_mil_graphics_d3d
//      $Keywords:
//
//  $Classes:
//      CD3DVertexXYZ*  - struct representations of common vertex formats
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

//  Note: WpfGfx uses only four vertex formats (see the history below). Each
//        maps to one ID3D11InputLayout; MilVertexFormatId is the key used by
//        the render state manager to pick the layout and the fixed function
//        emulation vertex shader.
//
//  9/10/2002 chrisra Removed the following classes:
//
//      CD3DVertexXYZW      - base class for CD3DVertexXYZW* that serve
//                            as members in Direct3D vertex buffer arrays.
//      CD3DVertexXYZWD
//      CD3DVertexXYZWUV
//      CD3DVertexXYZWDUV
//      CD3DVertexXYZWDSUV
//      CD3DVertexXYZWUV2
//      CD3DVertexXYZWUV3
//      CD3DVertexXYZWUV4
//
//      We're moving over to non-transformed vertices so everything with a W component
//      was removed.  In addition we've found major performance wins by using 1 vertex
//      format.  The reason is everytime we change we force D3D to flush all the vertices
//      stored up in their buffer.  This results in less vertices sent down to the card
//      each time, resulting in major perf loss.  I've replaced all the vertices from before
//      with the following 2 classes:
//
//      CD3DVertexXYZDUV2
//      CD3DVertexXYZDUV6
//
//      We found that vertex formats < 32 bytes performed about the same as those that were
//      32 bytes.  The next best performer is then 64 bytes.  Therefore only 2 different vertex
//      formats should be used, the 32 byte one that can hold position, color, and 2 texture stages,
//      and the 64 byte version for the cases when we need more than 2 texture coordinates.
//      At this point it doesn't make sense to have other formats.
//
//
//  2/24/2003 chrisra Added the following classes:
//
//      CD3DVertexXYZNDSUV4
//
//      Unless we're using bumpmapping we need to specify normals on the vertices to get decent
//      lighting.  For now we're adding this format for all 3D rendering.  Currently 2D will use
//      one of the 2 formats listed above: CD3DVertexXYZDUV2 or CD3DVertexXYZDUV6, and 3D will use
//      this new format.
//
//------------------------------------------------------------------------


//
// Vertex format identifiers. Replaces the D3D9 FVF code. The value is also the
// index into the input layout cache.
//
enum MilVertexFormatId
{
    MilVertexFormat_XYZDUV2    = 0,     // position, diffuse, 2 texture coordinates (32 bytes)
    MilVertexFormat_XYZDUV6    = 1,     // position, diffuse, 6 texture coordinates (64 bytes)
    MilVertexFormat_XYZDUV8    = 2,     // position, diffuse, 8 texture coordinates (80 bytes)
    MilVertexFormat_XYZNDSUV4  = 3,     // position, normal, diffuse, specular, 4 texture coordinates (64 bytes)
    MilVertexFormat_XYZDUV1    = 4,     // position, diffuse, 1 texture coordinate (24 bytes, 3D meshes)
    MilVertexFormat_XYZNUV1    = 5,     // position, normal, 1 texture coordinate (32 bytes, 3D meshes)

    MilVertexFormat_Count      = 6,
    MilVertexFormat_Unknown    = 0xFFFFFFFF
};

//
// NUM_OF_VERTEX_TEXTURE_COORDS - expands to number of texture coordinate sets
//  specified by given vertex type.  To work given type must have an array of
//  ptTx.
//
#define NUM_OF_VERTEX_TEXTURE_COORDS(VertexType) \
    ARRAYSIZE(((VertexType*)0)->ptTx)


// class CD3DVertexXYZDUV2
//
// 32 byte vertex format that can hold X,Y,Z position, Diffuse color, and 2 texture stages.
// This vertex format should be used for nearly all of our work, the only exceptions should
// be when more than 2 texture stages are required, and then CD3DVertexXYZDUV6, our 64 byte format,
// should be used.  No other formats should be used to minimize FVF switches in D3D.
class CD3DVertexXYZDUV2
{
public:
    union {
        struct {
            FLOAT X,Y,Z;
        };
        MilPoint2F ptPt;
    };
    DWORD Diffuse;
    union {
        struct {
            FLOAT U0, V0;
            FLOAT U1, V1;
        };
        MilPoint2F ptTx[2];
    };

    enum {Format = MilVertexFormat_XYZDUV2};

    __outro_ecount(1) const MilPoint2F &UV0() const { return ptTx[0]; }
    __outro_ecount(1) const MilPoint2F &UV1() const { return ptTx[1]; }

    void UV0(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[0] = ptUV; }
    void UV1(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[1] = ptUV; }

    MIL_FORCEINLINE void SetXYUV0( float newX, float newY, float newU0, float newV0 )
    {
        X = newX;
        Y = newY;
        Z = 0.5f;
        Diffuse = 0xff000000;
        U0 = newU0;
        V0 = newV0;
    }

    MIL_FORCEINLINE void SetXYUV1( float newX, float newY, float newU0, float newV0, float newU1, float newV1 )
    {
        X = newX;
        Y = newY;
        Z = 0.5f;
        Diffuse = 0xff000000;
        U0 = newU0;
        V0 = newV0;
        U1 = newU1;
        V1 = newV1;
    }

    MIL_FORCEINLINE void SetXYDUV0( float newX, float newY, D3DCOLOR newDiffuse, float newU0, float newV0 )
    {
        X = newX;
        Y = newY;
        Z = 0.5f;
        Diffuse = newDiffuse;
        U0 = newU0;
        V0 = newV0;
    }

};

// class CD3DVertexXYZNDSUV4
//
// 64 byte vertex format that stores position, Normal, Diffuse, Specular, and 4 textures samples.  Should only be
// used for 3D data.
class CD3DVertexXYZNDSUV4
{
public:
    union {
        struct {
            FLOAT X,Y,Z;
        };
        MilPoint2F ptPt;
    };
    union {
        struct {
            FLOAT Nx, Ny, Nz;
        };
        D3DVECTOR Normal;
    };
    DWORD Diffuse;
    DWORD Specular;
    union {
        struct {
            FLOAT U0, V0;
            FLOAT U1, V1;
            FLOAT U2, V2;
            FLOAT U3, V3;
        };
        MilPoint2F ptTx[4];
    };

    enum {Format = MilVertexFormat_XYZNDSUV4};

    MIL_FORCEINLINE __outro_ecount(1) const D3DVECTOR &GetNormal() const { return Normal; }
    MIL_FORCEINLINE void SetNormal(__in_ecount(1) const D3DVECTOR &_Normal) { Normal = _Normal; }
    MIL_FORCEINLINE __outro_ecount(1) const DWORD &GetSpecular() const { return Specular; }
    MIL_FORCEINLINE void SetSpecular(__in_ecount(1) const DWORD &_Specular) { Specular = _Specular; }


    __outro_ecount(1) const MilPoint2F &UV0() const { return ptTx[0]; }
    __outro_ecount(1) const MilPoint2F &UV1() const { return ptTx[1]; }
    __outro_ecount(1) const MilPoint2F &UV2() const { return ptTx[2]; }
    __outro_ecount(1) const MilPoint2F &UV3() const { return ptTx[3]; }

    void UV0(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[0] = ptUV; }
    void UV1(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[1] = ptUV; }
    void UV2(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[2] = ptUV; }
    void UV3(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[3] = ptUV; }

};

// class CD3DVertexXYZDUV6
//
// 64 byte vertex format that stores position, Diffuse, and 6 textures stages.  Should only be
// used when the 32 byte CD3DVertexXYZDUV2 is insufficient in it's number of texture stages.
class CD3DVertexXYZDUV6
{
public:
    union {
        struct {
            FLOAT X,Y,Z;
        };
        MilPoint2F ptPt;
    };
    DWORD Diffuse;
    union {
        struct {
            float U0, V0;
            float U1, V1;
            float U2, V2;
            float U3, V3;
            float U4, V4;
            float U5, V5;
        };
        MilPoint2F ptTx[6];
    };

    enum {Format = MilVertexFormat_XYZDUV6};

    __outro_ecount(1) const MilPoint2F &UV0() const { return ptTx[0]; }
    __outro_ecount(1) const MilPoint2F &UV1() const { return ptTx[1]; }
    __outro_ecount(1) const MilPoint2F &UV2() const { return ptTx[2]; }
    __outro_ecount(1) const MilPoint2F &UV3() const { return ptTx[3]; }
    __outro_ecount(1) const MilPoint2F &UV4() const { return ptTx[4]; }
    __outro_ecount(1) const MilPoint2F &UV5() const { return ptTx[5]; }

    void UV0(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[0] = ptUV; }
    void UV1(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[1] = ptUV; }
    void UV2(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[2] = ptUV; }
    void UV3(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[3] = ptUV; }
    void UV4(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[4] = ptUV; }
    void UV5(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[5] = ptUV; }


    MIL_FORCEINLINE void SetXYUV2(float newX, float newY, float newU0, float newV0, float newU1, float newV1,
                   float newU2, float newV2 )
        {
            X = newX;
            Y = newY;
            Z = 0.5f;
            Diffuse = 0xff000000;
            U0 = newU0;
            V0 = newV0;
            U1 = newU1;
            V1 = newV1;
            U2 = newU2;
            V2 = newV2;
        }

};

//
// class CD3DVertexXYZDUV8
//
// Large vertex format used in high quality blur.  Stores vertices position,
// Diffuse, and 8 textures stages.
//
struct CD3DVertexXYZDUV8
{
    enum {Format = MilVertexFormat_XYZDUV8};

    union {
        struct {
            FLOAT X,Y,Z;
        };
        MilPoint2F ptPt;
    };

    DWORD Diffuse;

    union {
        struct {
            float U0, V0;
            float U1, V1;
            float U2, V2;
            float U3, V3;
            float U4, V4;
            float U5, V5;
            float U6, V6;
            float U7, V7;
        };
        MilPoint2F ptTx[8];
    };

    __outro_ecount(1) const MilPoint2F &UV0() const { return ptTx[0]; }
    __outro_ecount(1) const MilPoint2F &UV1() const { return ptTx[1]; }
    __outro_ecount(1) const MilPoint2F &UV2() const { return ptTx[2]; }
    __outro_ecount(1) const MilPoint2F &UV3() const { return ptTx[3]; }
    __outro_ecount(1) const MilPoint2F &UV4() const { return ptTx[4]; }
    __outro_ecount(1) const MilPoint2F &UV5() const { return ptTx[5]; }
    __outro_ecount(1) const MilPoint2F &UV6() const { return ptTx[6]; }
    __outro_ecount(1) const MilPoint2F &UV7() const { return ptTx[7]; }

    void UV0(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[0] = ptUV; }
    void UV1(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[1] = ptUV; }
    void UV2(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[2] = ptUV; }
    void UV3(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[3] = ptUV; }
    void UV4(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[4] = ptUV; }
    void UV5(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[5] = ptUV; }
    void UV6(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[6] = ptUV; }
    void UV7(__in_ecount(1) const MilPoint2F &ptUV) { ptTx[7] = ptUV; }
};

//+-----------------------------------------------------------------------------
//
//  Function:  MilVertexFormatStride
//
//  Synopsis:  Vertex size in bytes for a vertex format id.
//
//------------------------------------------------------------------------------
MIL_FORCEINLINE UINT
MilVertexFormatStride(MilVertexFormatId format)
{
    switch (format)
    {
    case MilVertexFormat_XYZDUV2:   return sizeof(CD3DVertexXYZDUV2);
    case MilVertexFormat_XYZDUV6:   return sizeof(CD3DVertexXYZDUV6);
    case MilVertexFormat_XYZDUV8:   return sizeof(CD3DVertexXYZDUV8);
    case MilVertexFormat_XYZNDSUV4: return sizeof(CD3DVertexXYZNDSUV4);
    case MilVertexFormat_XYZDUV1:   return 12 + 4 + 8;
    case MilVertexFormat_XYZNUV1:   return 12 + 12 + 8;
    default:
        Assert(false);
        return 0;
    }
}

//+-----------------------------------------------------------------------------
//
//  Function:  MilVertexFormatTexCoordCount
//
//  Synopsis:  Number of texture coordinate sets in a vertex format.
//
//------------------------------------------------------------------------------
MIL_FORCEINLINE UINT
MilVertexFormatTexCoordCount(MilVertexFormatId format)
{
    switch (format)
    {
    case MilVertexFormat_XYZDUV2:   return 2;
    case MilVertexFormat_XYZDUV6:   return 6;
    case MilVertexFormat_XYZDUV8:   return 8;
    case MilVertexFormat_XYZNDSUV4: return 4;
    case MilVertexFormat_XYZDUV1:   return 1;
    case MilVertexFormat_XYZNUV1:   return 1;
    default:
        Assert(false);
        return 0;
    }
}

//+-----------------------------------------------------------------------------
//
//  Function:  MilVertexFormatHasNormal / MilVertexFormatHasDiffuse / HasSpecular
//
//------------------------------------------------------------------------------
MIL_FORCEINLINE bool
MilVertexFormatHasNormal(MilVertexFormatId format)
{
    return format == MilVertexFormat_XYZNDSUV4 || format == MilVertexFormat_XYZNUV1;
}

MIL_FORCEINLINE bool
MilVertexFormatHasDiffuse(MilVertexFormatId format)
{
    return format != MilVertexFormat_XYZNUV1;
}

MIL_FORCEINLINE bool
MilVertexFormatHasSpecular(MilVertexFormatId format)
{
    return format == MilVertexFormat_XYZNDSUV4;
}
