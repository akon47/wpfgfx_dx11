using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Effects;

namespace SmokeApp;

/// <summary>
/// Compiles HLSL to legacy D3D9 pixel shader bytecode at run time with d3dcompiler_47.dll
/// (part of Windows), so the test needs no prebuilt .ps files. ShaderEffect accepts exactly
/// this bytecode on the stock DX9 engine; the DX11 engine must translate it (D6, M8).
/// </summary>
internal static class LegacyShaderCompiler
{
    [DllImport("d3dcompiler_47.dll", CharSet = CharSet.Ansi, ExactSpelling = true)]
    private static extern int D3DCompile(
        byte[] srcData, nuint srcDataSize, string? sourceName, IntPtr defines, IntPtr include,
        string entryPoint, string target, uint flags1, uint flags2, out IntPtr code, out IntPtr errorMsgs);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate IntPtr GetBufferPointerFn(IntPtr blob);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate nuint GetBufferSizeFn(IntPtr blob);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    private delegate uint ReleaseFn(IntPtr blob);

    private static byte[] ReadBlob(IntPtr blob)
    {
        IntPtr vtable = Marshal.ReadIntPtr(blob);
        var release = Marshal.GetDelegateForFunctionPointer<ReleaseFn>(Marshal.ReadIntPtr(vtable, 2 * IntPtr.Size));
        var getPointer = Marshal.GetDelegateForFunctionPointer<GetBufferPointerFn>(Marshal.ReadIntPtr(vtable, 3 * IntPtr.Size));
        var getSize = Marshal.GetDelegateForFunctionPointer<GetBufferSizeFn>(Marshal.ReadIntPtr(vtable, 4 * IntPtr.Size));
        var bytes = new byte[(int)getSize(blob)];
        Marshal.Copy(getPointer(blob), bytes, 0, bytes.Length);
        release(blob);
        return bytes;
    }

    public static PixelShader Compile(string hlsl, string target)
    {
        byte[] src = Encoding.ASCII.GetBytes(hlsl);
        int hr = D3DCompile(src, (nuint)src.Length, null, IntPtr.Zero, IntPtr.Zero, "main", target, 0, 0, out IntPtr code, out IntPtr errors);
        if (hr < 0)
        {
            string message = errors != IntPtr.Zero ? Encoding.ASCII.GetString(ReadBlob(errors)) : "";
            throw new InvalidOperationException($"D3DCompile({target}) failed 0x{hr:x8}: {message}");
        }
        if (errors != IntPtr.Zero) ReadBlob(errors);

        var shader = new PixelShader();
        shader.SetStreamSource(new MemoryStream(ReadBlob(code)));
        return shader;
    }
}

/// <summary>ps_2_0: grayscale with a tint from a float4 register (c0).</summary>
public class TintEffect : ShaderEffect
{
    private const string Hlsl = @"
sampler2D input : register(s0);
float4 tint : register(c0);
float4 main(float2 uv : TEXCOORD) : COLOR
{
    float4 c = tex2D(input, uv);
    float g = dot(c.rgb, float3(0.3, 0.59, 0.11));
    return float4(lerp(g.xxx, tint.rgb * g, tint.a), c.a);
}";

    public static readonly DependencyProperty InputProperty =
        RegisterPixelShaderSamplerProperty("Input", typeof(TintEffect), 0);

    public static readonly DependencyProperty TintProperty =
        DependencyProperty.Register("Tint", typeof(Color), typeof(TintEffect),
            new UIPropertyMetadata(Color.FromArgb(255, 255, 160, 32), PixelShaderConstantCallback(0)));

    public TintEffect()
    {
        PixelShader = LegacyShaderCompiler.Compile(Hlsl, "ps_2_0");
        UpdateShaderValue(InputProperty);
        UpdateShaderValue(TintProperty);
    }

    public Brush Input { get => (Brush)GetValue(InputProperty); set => SetValue(InputProperty, value); }
    public Color Tint { get => (Color)GetValue(TintProperty); set => SetValue(TintProperty, value); }
}

/// <summary>
/// ps_3_0: flow control (loop with a literal trip count, if/else on a register), several
/// texture reads, a float register (c0.x = split position, c0.y = sample step).
/// </summary>
public class SplitBlurEffect : ShaderEffect
{
    private const string Hlsl = @"
sampler2D input : register(s0);
float4 p : register(c0);
float4 main(float2 uv : TEXCOORD) : COLOR
{
    float4 acc = 0;
    [loop] for (int i = -3; i <= 3; i++)
    {
        acc += tex2D(input, uv + float2(i * p.y, 0));
    }
    acc /= 7;
    if (uv.x > p.x)
    {
        return float4(1 - acc.rgb * acc.a, acc.a);
    }
    else
    {
        return acc;
    }
}";

    public static readonly DependencyProperty InputProperty =
        RegisterPixelShaderSamplerProperty("Input", typeof(SplitBlurEffect), 0);

    public static readonly DependencyProperty ParamsProperty =
        DependencyProperty.Register("Params", typeof(Point), typeof(SplitBlurEffect),
            new UIPropertyMetadata(new Point(0.5, 0.012), PixelShaderConstantCallback(0)));

    public SplitBlurEffect()
    {
        PixelShader = LegacyShaderCompiler.Compile(Hlsl, "ps_3_0");
        UpdateShaderValue(InputProperty);
        UpdateShaderValue(ParamsProperty);
    }

    public Brush Input { get => (Brush)GetValue(InputProperty); set => SetValue(InputProperty, value); }
    public Point Params { get => (Point)GetValue(ParamsProperty); set => SetValue(ParamsProperty, value); }
}
