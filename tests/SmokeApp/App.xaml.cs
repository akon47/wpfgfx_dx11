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
    static App()
    {
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
}
