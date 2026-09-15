# sd_fatfs — FatFs on the BeagleBone Black microSD slot

Mounts the microSD card on MMC0 with ChaN's FatFs, lists the root directory, writes a small text
file and reads it back. Everything is reported through the RTT log.

## Prerequisite: vendored FatFs

The example expects **FatFs R0.16** unpacked as shipped under `thirdparty/ff16/`:

```
thirdparty/ff16/source/ff.c
thirdparty/ff16/source/ff.h
thirdparty/ff16/source/ffunicode.c
thirdparty/ff16/source/diskio.h
```

Get it from <http://elm-chan.org/fsw/ff/>. While those files are missing, `examples/CMakeLists.txt`
prints a notice and skips this example, so the rest of the tree still builds.

Only `ff.c` and `ffunicode.c` are compiled. `diskio.c` and `ffsystem.c` are ignored: the disk port
lives in `src/diskio.cpp`, and no heap or mutex hooks are needed because `FF_USE_LFN` is 1 (static
LFN buffer) and `FF_FS_REENTRANT` is 0.

## How the configuration is injected

The vendored tree keeps its own `source/ffconf.h`, and `ff.h` includes it with quotes — which
resolves next to `ff.h`, so an include path alone cannot override it. R0.16 guards that include:

```c
#if !defined(FFCONF_DEF)
#include "ffconf.h"
#endif
```

So the build force-includes the project's configuration instead, and the vendored tree is never
modified:

```cmake
target_compile_options(${SD_FATFS_TARGET} PRIVATE
        "-include${CMAKE_CURRENT_SOURCE_DIR}/src/ffconf.h")
```

`src/ffconf.h` sets code page 437, long file names with a static buffer, `f_gets`/`f_printf`, a
fixed timestamp instead of an RTC, and **no `f_mkfs`** — the example can never format the card.

## What it needs from the card

A card with an MBR and a FAT12/16/32 partition, i.e. an ordinary card as formatted by Windows,
macOS or `mkfs.vfat`. exFAT is disabled, so a card formatted as exFAT (common on 64 GB and larger
media out of the box) is reported as `FR_NO_FILESYSTEM` and must be reformatted as FAT32.

## What it writes

One file, `0:/AM335X.TXT`, created with `FA_CREATE_ALWAYS` and about 90 bytes long. Nothing else on
the card is touched. The file is left in place so it can be inspected on a PC afterwards.

## Related

`examples/sd_blocks` exercises the same driver one layer lower, straight on the 512-byte block
interface, with no file system involved. Start there when a card is not recognized at all.
