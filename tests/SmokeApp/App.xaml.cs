using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows;

namespace SmokeApp;

public partial class App : Application
{
    /// <summary>
    /// --wpfgfx &lt;dir&gt; redirects every P/Invoke into wpfgfx_cor3.dll to the copy in that
    /// directory. Installed in the static constructor so it runs before any WPF assembly
    /// resolves the native engine.
    /// </summary>
    /// <summary>
    /// Managed call stacks captured when d3d9.dll was loaded into the process (empty when it
    /// never loaded). Lets the report show who pulled the D3D9 runtime in.
    /// </summary>
    public static readonly List<string> D3D9LoadStacks = new();

    private static LdrDllNotification? s_ldrCallback; // keep the delegate alive

    /// <summary>What the loader watch saw when it was installed (static constructor time).</summary>
    public static string D3D9WatchStatus = "";

    static App()
    {
        RegisterD3D9LoadWatch();

        string[] args = Environment.GetCommandLineArgs();
        for (int i = 1; i < args.Length; i++)
        {
            if (args[i] == "--wpfgfx" && i + 1 < args.Length)
            {
                string dir = Path.GetFullPath(args[i + 1]);
                DllImportResolver resolver = (name, _, _) =>
                {
                    if (!name.StartsWith("wpfgfx", StringComparison.OrdinalIgnoreCase)) return IntPtr.Zero;
                    string candidate = Path.Combine(dir, name.EndsWith(".dll", StringComparison.OrdinalIgnoreCase) ? name : name + ".dll");
                    return File.Exists(candidate) ? NativeLibrary.Load(candidate) : IntPtr.Zero;
                };
                // Every WPF assembly that declares DllImport(wpfgfx_cor3.dll).
                NativeLibrary.SetDllImportResolver(typeof(System.Windows.Media.Visual).Assembly, resolver);      // PresentationCore
                NativeLibrary.SetDllImportResolver(typeof(System.Windows.Window).Assembly, resolver);            // PresentationFramework
                NativeLibrary.SetDllImportResolver(typeof(System.Windows.Threading.Dispatcher).Assembly, resolver); // WindowsBase
            }
        }
    }

    private static void RegisterD3D9LoadWatch()
    {
        // Warm up the stack trace machinery so nothing new has to load inside the loader callback.
        _ = new System.Diagnostics.StackTrace(false).GetFrames();

        s_ldrCallback = (reason, data, _) =>
        {
            if (reason != 1 /* LDR_DLL_NOTIFICATION_REASON_LOADED */) return;
            // LDR_DLL_LOADED_NOTIFICATION_DATA: ULONG Flags; PCUNICODE_STRING FullDllName; PCUNICODE_STRING BaseDllName; ...
            IntPtr baseName = Marshal.ReadIntPtr(data, 2 * IntPtr.Size);
            ushort length = (ushort)Marshal.ReadInt16(baseName, 0);
            IntPtr buffer = Marshal.ReadIntPtr(baseName, IntPtr.Size);
            string name = Marshal.PtrToStringUni(buffer, length / 2);
            if (!name.Equals("d3d9.dll", StringComparison.OrdinalIgnoreCase)) return;

            var frames = new System.Diagnostics.StackTrace(false).GetFrames();
            var sb = new System.Text.StringBuilder();
            foreach (var f in frames)
            {
                var m = f.GetMethod();
                if (m == null) continue;
                sb.Append(m.DeclaringType?.FullName).Append('.').Append(m.Name).Append(" <- ");
            }
            lock (D3D9LoadStacks) D3D9LoadStacks.Add(sb.ToString());
        };
        int status = LdrRegisterDllNotification(0, s_ldrCallback, IntPtr.Zero, out _);
        bool alreadyLoaded = false;
        foreach (System.Diagnostics.ProcessModule m in System.Diagnostics.Process.GetCurrentProcess().Modules)
            if (m.ModuleName.Equals("d3d9.dll", StringComparison.OrdinalIgnoreCase)) alreadyLoaded = true;
        D3D9WatchStatus = $"watch registered (status=0x{status:x8}); d3d9.dll already loaded at startup: {alreadyLoaded}";
    }

    private delegate void LdrDllNotification(uint reason, IntPtr data, IntPtr context);

    [DllImport("ntdll.dll")]
    private static extern int LdrRegisterDllNotification(uint flags, LdrDllNotification callback, IntPtr context, out IntPtr cookie);
}
