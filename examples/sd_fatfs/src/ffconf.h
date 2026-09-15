/*---------------------------------------------------------------------------/
/  FatFs Functional Configurations for the AM335x BSP sd_fatfs example
/
/  Targets ChaN's FatFs R0.16, vendored under thirdparty/ff16. ff.h compares this
/  revision stamp with its own and refuses to build on a mismatch.
/
/  ff.h only includes its own ffconf.h when FFCONF_DEF is still undefined, so the
/  build force-includes this file (-include) instead of copying it into the
/  vendored tree. See examples/sd_fatfs/CMakeLists.txt.
/---------------------------------------------------------------------------*/

#ifndef FFCONF_AM335X_H
#define FFCONF_AM335X_H

#define FFCONF_DEF  80386   /* Revision ID of FatFs R0.16 */

/*---------------------------------------------------------------------------/
/ Function Configurations
/---------------------------------------------------------------------------*/

#define FF_FS_READONLY  0
/* The example writes a file, so the write side of FatFs is needed. */

#define FF_FS_MINIMIZE  0
/* Full API: f_stat, f_opendir, f_readdir, f_getfree, f_unlink and friends. */

#define FF_USE_FIND     0
#define FF_USE_MKFS     0
/* Formatting is deliberately off: the example must never be able to wipe the card. */

#define FF_USE_FASTSEEK 0
#define FF_USE_EXPAND   0
#define FF_USE_CHMOD    0
#define FF_USE_LABEL    0
#define FF_USE_FORWARD  0

#define FF_USE_STRFUNC  1
#define FF_PRINT_LLI    0
#define FF_PRINT_FLOAT  0
#define FF_STRF_ENCODE  0
/* f_puts / f_gets / f_printf, used by the example to write a small text file. */

/*---------------------------------------------------------------------------/
/ Locale and Namespace Configurations
/---------------------------------------------------------------------------*/

#define FF_CODE_PAGE    437
/* U.S. code page. Needs ffunicode.c in the build. */

#define FF_USE_LFN      1
/* 1 = long file names with a static working buffer in .bss: no heap is required,
   which suits a baremetal target. The API is not reentrant in this mode, which is
   fine because FF_FS_REENTRANT is 0. */
#define FF_MAX_LFN      255
#define FF_LFN_UNICODE  0
#define FF_LFN_BUF      255
#define FF_SFN_BUF      12

#define FF_FS_RPATH     0
/* No relative paths: every path the example uses is absolute. */

#define FF_PATH_DEPTH   10
/* Only consulted when FF_FS_RPATH is 2; kept at the upstream default. */

/*---------------------------------------------------------------------------/
/ Drive/Volume Configurations
/---------------------------------------------------------------------------*/

#define FF_VOLUMES      1
/* A single volume, physical drive 0, the microSD slot on MMC0. */

#define FF_STR_VOLUME_ID    0
#define FF_VOLUME_STRS      "RAM","NAND","CF","SD","SD2","USB","USB2","USB3"

#define FF_MULTI_PARTITION  0
/* Drive 0 maps to the first partition found in the MBR. */

#define FF_MIN_SS       512
#define FF_MAX_SS       512
/* Every SD card exposes 512-byte blocks, so the sector size is fixed and
   disk_ioctl(GET_SECTOR_SIZE) is never called. */

#define FF_LBA64        0
/* 32-bit LBA is enough: it covers cards up to 2 TiB. */

#define FF_MIN_GPT      0x10000000
#define FF_USE_TRIM     0

/*---------------------------------------------------------------------------/
/ System Configurations
/---------------------------------------------------------------------------*/

#define FF_FS_TINY      0
/* Each FIL keeps its own sector buffer: faster, at the price of 512 bytes per file. */

#define FF_FS_EXFAT     0
/* FAT12/16/32 only. exFAT would also drag in a licence obligation. */

#define FF_FS_NORTC     1
#define FF_NORTC_MON    1
#define FF_NORTC_MDAY   1
#define FF_NORTC_YEAR   2025
/* The board has no wired RTC in this example, so FatFs stamps every file with the
   fixed date above instead of calling get_fattime(). */

#define FF_FS_CRTIME    0
/* No creation timestamp in FILINFO: there is no RTC behind it in this example. */

#define FF_FS_NOFSINFO  0
#define FF_FS_LOCK      0

#define FF_FS_REENTRANT 0
#define FF_FS_TIMEOUT   1000
/* Baremetal, single threaded: no mutex hooks and therefore no ffsystem.c. */

#endif /* FFCONF_AM335X_H */
