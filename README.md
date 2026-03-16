# MKernelPatch

**Patching and hooking the Linux kernel with only a stripped Linux kernel image — now with ARMv7 support.**

``` shell
 __  __  _  __                    _ ____       _       _     
|  \/  || |/ /___ _ __ _ __   ___| |  _ \ __ _| |_ ___| |__  
| |\/| || ' // _ \ '__| '_ \ / _ \ | |_) / _` | __/ __| '_ \ 
| |  | || . \  __/ |  | | | |  __/ |  __/ (_| | || (__| | | |
|_|  |_||_|\_\___|_|  |_| |_|\___|_|_|   \__,_|\__\___|_| |_|

```

- Obtain all symbol information without source code and symbol information.
- Inject arbitrary code into the kernel. (Static patching the kernel image or Runtime dynamic loading).
- Kernel function inline hook and syscall table hook are provided.
- Additional SU for Android.
- **ARMv7 (armeabi-v7a) support** — bringing kernel patching to 32-bit devices.

If you are using Android, [MPatch](https://github.com/matheusc457/MPatch) would be a better choice.

## Requirement

CONFIG_KALLSYMS=y  

## Supported Versions

Supports **arm64** and **ARMv7 (armeabi-v7a)** architectures.

> ARMv7 support is currently under active development.

Linux 3.18 - 6.6 (theoretically)  

## Get Involved

## More Information

[Documentation](./doc/)

## Credits

- [bmax121](https://github.com/bmax121) — KernelPatch original author
- [vmlinux-to-elf](https://github.com/marin-m/vmlinux-to-elf): Some ideas for parsing kernel symbols.
- [android-inline-hook](https://github.com/bytedance/android-inline-hook): Some code for fixing arm64 inline hook instructions.
- [tlsf](https://github.com/mattconte/tlsf): Memory allocator used for KPM. (Need another to allocate ROX memory.)

## License

MKernelPatch is licensed under the **GNU General Public License (GPL) 2.0** (<https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>).
