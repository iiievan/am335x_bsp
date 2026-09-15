#include <cstddef>
#include <cstdint>

#include "sd_fatfs.h"

#include "ff.h"
#include "log/log.h"

#define TAG "fatfs"

namespace
{
    /*  FatFs objects are large, so they live in .bss rather than on the application stack. */
    FATFS g_fs;
    FIL   g_file;
    DIR   g_dir;
    FILINFO g_info;

    constexpr const char* TEST_PATH = "0:/AM335X.TXT";

    char g_line[128];

    const char* fs_type_name(const BYTE type) noexcept
    {
        switch (type)
        {
            case FS_FAT12: return "FAT12";
            case FS_FAT16: return "FAT16";
            case FS_FAT32: return "FAT32";
            case FS_EXFAT: return "exFAT";
            default:       return "unknown";
        }
    }

    const char* fr_name(const FRESULT fr) noexcept
    {
        switch (fr)
        {
            case FR_OK:                  return "FR_OK";
            case FR_DISK_ERR:            return "FR_DISK_ERR";
            case FR_INT_ERR:             return "FR_INT_ERR";
            case FR_NOT_READY:           return "FR_NOT_READY";
            case FR_NO_FILE:             return "FR_NO_FILE";
            case FR_NO_PATH:             return "FR_NO_PATH";
            case FR_INVALID_NAME:        return "FR_INVALID_NAME";
            case FR_DENIED:              return "FR_DENIED";
            case FR_EXIST:               return "FR_EXIST";
            case FR_INVALID_OBJECT:      return "FR_INVALID_OBJECT";
            case FR_WRITE_PROTECTED:     return "FR_WRITE_PROTECTED";
            case FR_INVALID_DRIVE:       return "FR_INVALID_DRIVE";
            case FR_NOT_ENABLED:         return "FR_NOT_ENABLED";
            case FR_NO_FILESYSTEM:       return "FR_NO_FILESYSTEM";
            case FR_MKFS_ABORTED:        return "FR_MKFS_ABORTED";
            case FR_TIMEOUT:             return "FR_TIMEOUT";
            case FR_LOCKED:              return "FR_LOCKED";
            case FR_NOT_ENOUGH_CORE:     return "FR_NOT_ENOUGH_CORE";
            case FR_TOO_MANY_OPEN_FILES: return "FR_TOO_MANY_OPEN_FILES";
            case FR_INVALID_PARAMETER:   return "FR_INVALID_PARAMETER";
            default:                     return "FR_?";
        }
    }

    [[nodiscard]] bool list_root() noexcept
    {
        FRESULT fr = f_opendir(&g_dir, "0:/");
        if (fr != FR_OK)
        {
            LOG_E(TAG, "f_opendir failed: %s", fr_name(fr));
            return false;
        }

        LOG_I(TAG, "--- root directory ---");

        uint32_t entries = 0u;

        for (;;)
        {
            fr = f_readdir(&g_dir, &g_info);

            if (fr != FR_OK)
            {
                LOG_E(TAG, "f_readdir failed: %s", fr_name(fr));
                (void)f_closedir(&g_dir);
                return false;
            }

            if (g_info.fname[0] == '\0')
                break;      /* end of the directory */

            if ((g_info.fattrib & AM_DIR) != 0u)
                LOG_I(TAG, "  <DIR>  %s", g_info.fname);
            else
                LOG_I(TAG, "  %8lu %s", static_cast<unsigned long>(g_info.fsize), g_info.fname);

            ++entries;
        }

        (void)f_closedir(&g_dir);

        LOG_I(TAG, "%lu entrie(s) in the root directory", static_cast<unsigned long>(entries));
        return true;
    }

    [[nodiscard]] bool write_then_read_back() noexcept
    {
        static const char payload[] =
            "Written by the AM335x BSP sd_fatfs example.\r\n"
            "MMC0 -> microSD, 4-bit bus, FatFs R0.15.\r\n";

        LOG_I(TAG, "--- writing %s ---", TEST_PATH);

        FRESULT fr = f_open(&g_file, TEST_PATH, FA_WRITE | FA_CREATE_ALWAYS);
        if (fr != FR_OK)
        {
            LOG_E(TAG, "f_open for writing failed: %s", fr_name(fr));
            return false;
        }

        UINT written = 0u;
        const UINT to_write = static_cast<UINT>(sizeof(payload) - 1u);

        fr = f_write(&g_file, payload, to_write, &written);

        const FRESULT closed = f_close(&g_file);

        if (fr != FR_OK)
        {
            LOG_E(TAG, "f_write failed: %s", fr_name(fr));
            return false;
        }

        if (written != to_write)
        {
            LOG_E(TAG, "short write: %lu of %lu bytes",
                  static_cast<unsigned long>(written), static_cast<unsigned long>(to_write));
            return false;
        }

        if (closed != FR_OK)
        {
            LOG_E(TAG, "f_close failed: %s", fr_name(closed));
            return false;
        }

        LOG_I(TAG, "%lu bytes written", static_cast<unsigned long>(written));

        LOG_I(TAG, "--- reading %s back ---", TEST_PATH);

        fr = f_open(&g_file, TEST_PATH, FA_READ);
        if (fr != FR_OK)
        {
            LOG_E(TAG, "f_open for reading failed: %s", fr_name(fr));
            return false;
        }

        LOG_I(TAG, "file size: %lu bytes", static_cast<unsigned long>(f_size(&g_file)));

        bool ok = true;

        while (f_gets(g_line, static_cast<int>(sizeof(g_line)), &g_file) != nullptr)
        {
            /* Trim the line ending so the log stays on one line per record */
            for (size_t i = 0; i < sizeof(g_line); ++i)
            {
                if (g_line[i] == '\0')
                    break;

                if ((g_line[i] == '\r') || (g_line[i] == '\n'))
                {
                    g_line[i] = '\0';
                    break;
                }
            }

            LOG_I(TAG, "  | %s", g_line);
        }

        if (f_error(&g_file) != 0)
        {
            LOG_E(TAG, "an error occurred while reading the file back");
            ok = false;
        }

        (void)f_close(&g_file);

        return ok;
    }
}

bool sd_fatfs_test(void)
{
    /*  The second argument of f_mount is 1: mount right now rather than lazily, so that a card that
     *  cannot be read is reported here instead of at the first f_open.
     */
    FRESULT fr = f_mount(&g_fs, "0:", 1u);

    if (fr != FR_OK)
    {
        LOG_E(TAG, "f_mount failed: %s", fr_name(fr));

        if (fr == FR_NO_FILESYSTEM)
            LOG_E(TAG, "the card carries no FAT volume - format it on a PC first");

        return false;
    }

    LOG_I(TAG, "mounted: %s, cluster size %lu sectors, %lu clusters",
          fs_type_name(g_fs.fs_type),
          static_cast<unsigned long>(g_fs.csize),
          static_cast<unsigned long>(g_fs.n_fatent - 2u));

    DWORD free_clusters = 0u;
    FATFS* fsp = nullptr;

    fr = f_getfree("0:", &free_clusters, &fsp);
    if (fr != FR_OK)
    {
        LOG_E(TAG, "f_getfree failed: %s", fr_name(fr));
        return false;
    }

    const uint64_t total_kib =
        (static_cast<uint64_t>(fsp->n_fatent - 2u) * fsp->csize * FF_MAX_SS) / 1024u;
    const uint64_t free_kib =
        (static_cast<uint64_t>(free_clusters) * fsp->csize * FF_MAX_SS) / 1024u;

    LOG_I(TAG, "volume: %lu KiB total, %lu KiB free",
          static_cast<unsigned long>(total_kib),
          static_cast<unsigned long>(free_kib));

    if (!list_root())
        return false;

    if (!write_then_read_back())
        return false;

    /*  List the root once more so the new file shows up. */
    if (!list_root())
        return false;

    (void)f_mount(nullptr, "0:", 0u);

    return true;
}
