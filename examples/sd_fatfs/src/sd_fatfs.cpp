#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "sd_fatfs.h"
#include "ff.h"
#include "diskio.h"
#include "log/log.h"

#define TAG "fatfs"

namespace
{
    /* Static storage: the baremetal stack is small. Volumes are tested serially;
     * FF_USE_LFN=1 is intentionally single-threaded.
     */
    FATFS g_fs[FF_VOLUMES];
    FIL g_file;
    DIR g_dir;
    FILINFO g_info;
    alignas(4) BYTE g_sector[512];
    char g_payload[192];
    char g_readback[192];

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

    uint32_t le32(const BYTE* p) noexcept
    {
        return static_cast<uint32_t>(p[0]) |
               (static_cast<uint32_t>(p[1]) << 8) |
               (static_cast<uint32_t>(p[2]) << 16) |
               (static_cast<uint32_t>(p[3]) << 24);
    }

    [[nodiscard]] bool report_mbr() noexcept
    {
        if ((disk_initialize(0u) & STA_NOINIT) != 0u)
        {
            LOG_E(TAG, "MMC0 initialization failed");
            return false;
        }

        LBA_t sectors = 0u;
        if (disk_ioctl(0u, GET_SECTOR_COUNT, &sectors) != RES_OK ||
            disk_read(0u, g_sector, 0u, 1u) != RES_OK)
        {
            LOG_E(TAG, "cannot read card capacity / LBA 0");
            return false;
        }

        if (g_sector[510] != 0x55u || g_sector[511] != 0xAAu)
        {
            LOG_E(TAG, "LBA 0 has no 55AA signature; explicit MBR mapping requires an MBR");
            return false;
        }

        /* A 55AA signature alone does not prove this is a valid partition table.
         * Report all four entries; f_mount validates each requested filesystem.
         */
        for (unsigned int i = 0u; i < 4u; ++i)
        {
            const BYTE* const entry = g_sector + 446u + i * 16u;
            const uint32_t start = le32(entry + 8u);
            const uint32_t count = le32(entry + 12u);
            LOG_I(TAG, "MBR[%u]: type=0x%02x, start=%lu, sectors=%lu",
                  i + 1u, static_cast<unsigned int>(entry[4]),
                  static_cast<unsigned long>(start), static_cast<unsigned long>(count));

            if (count != 0u && (start >= sectors || count > sectors - start))
                LOG_W(TAG, "MBR[%u] extends outside the card", i + 1u);
            if (entry[4] == 0xEEu)
            {
                LOG_E(TAG, "protective MBR / GPT is not supported by this 32-bit MBR configuration");
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool list_root(const char* root) noexcept
    {
        FRESULT fr = f_opendir(&g_dir, root);
        if (fr != FR_OK)
        {
            LOG_E(TAG, "%s f_opendir: %s", root, fr_name(fr));
            return false;
        }

        LOG_I(TAG, "--- directory %s ---", root);
        uint32_t entries = 0u;
        bool ok = true;
        for (;;)
        {
            fr = f_readdir(&g_dir, &g_info);
            if (fr != FR_OK)
            {
                LOG_E(TAG, "%s f_readdir: %s", root, fr_name(fr));
                ok = false;
                break;
            }
            if (g_info.fname[0] == '\0')
                break;

            if ((g_info.fattrib & AM_DIR) != 0u)
                LOG_I(TAG, "  <DIR> %s", g_info.fname);
            else
                LOG_I(TAG, "  %llu bytes  %s",
                      static_cast<unsigned long long>(g_info.fsize), g_info.fname);
            ++entries;
        }

        fr = f_closedir(&g_dir);
        if (fr != FR_OK)
        {
            LOG_E(TAG, "%s f_closedir: %s", root, fr_name(fr));
            ok = false;
        }
        LOG_I(TAG, "%s %lu entries", root, static_cast<unsigned long>(entries));
        return ok;
    }

    [[nodiscard]] bool close_file(const char* path) noexcept
    {
        const FRESULT fr = f_close(&g_file);
        if (fr == FR_OK)
            return true;
        LOG_E(TAG, "%s f_close: %s", path, fr_name(fr));
        return false;
    }

    [[nodiscard]] bool write_then_read_back(const unsigned int volume) noexcept
    {
        /* A long UTF-8 name exercises LFN and Unicode on both FAT and exFAT.
         * FA_CREATE_NEW prevents accidental replacement of an existing file.
         */
        char path[96];
        FRESULT fr = FR_EXIST;
        for (unsigned int sequence = 0u; sequence < 1000u; ++sequence)
        {
            snprintf(path, sizeof(path), u8"%u:/AM335X_FatFs_тест_%03u.txt", volume, sequence);
            fr = f_open(&g_file, path, FA_WRITE | FA_CREATE_NEW);
            if (fr != FR_EXIST)
                break;
        }
        if (fr != FR_OK)
        {
            LOG_E(TAG, "%u: create test file: %s", volume, fr_name(fr));
            return false;
        }

        const int n = snprintf(g_payload, sizeof(g_payload),
                              "AM335x BSP / FatFs R0.16\r\n"
                              "Logical drive %u: / MMC0 / MBR partition %u\r\n"
                              "FAT12/16/32 + exFAT; UTF-8 filename; byte-for-byte verification.\r\n",
                              volume, static_cast<unsigned int>(VolToPart[volume].pt));
        if (n < 0 || static_cast<size_t>(n) >= sizeof(g_payload))
        {
            LOG_E(TAG, "payload formatting failed");
            (void)close_file(path);
            return false;
        }

        const UINT expected = static_cast<UINT>(n);
        UINT written = 0u;
        fr = f_write(&g_file, g_payload, expected, &written);
        const bool closed = close_file(path);
        if (fr != FR_OK || written != expected || !closed)
        {
            LOG_E(TAG, "%s write: %s, %u/%u bytes", path, fr_name(fr), written, expected);
            return false;
        }

        fr = f_open(&g_file, path, FA_READ);
        if (fr != FR_OK)
        {
            LOG_E(TAG, "%s reopen: %s", path, fr_name(fr));
            return false;
        }

        const bool size_ok = f_size(&g_file) == static_cast<FSIZE_t>(expected);
        UINT got = 0u;
        fr = f_read(&g_file, g_readback, expected, &got);
        const bool read_closed = close_file(path);
        if (fr != FR_OK || got != expected || !size_ok || !read_closed)
        {
            LOG_E(TAG, "%s read: %s, %u/%u bytes, size_ok=%u",
                  path, fr_name(fr), got, expected, size_ok ? 1u : 0u);
            return false;
        }

        for (UINT i = 0u; i < expected; ++i)
        {
            if (g_readback[i] != g_payload[i])
            {
                LOG_E(TAG, "%s mismatch at byte %u: expected=%02x, actual=%02x", path, i,
                      static_cast<unsigned int>(static_cast<unsigned char>(g_payload[i])),
                      static_cast<unsigned int>(static_cast<unsigned char>(g_readback[i])));
                return false;
            }
        }
        LOG_I(TAG, "%s: verified %u bytes; file left on card", path, expected);
        return true;
    }

    [[nodiscard]] bool test_volume(const unsigned int volume) noexcept
    {
        char drive[4];
        char root[5];
        snprintf(drive, sizeof(drive), "%u:", volume);
        snprintf(root, sizeof(root), "%u:/", volume);
        LOG_I(TAG, "--- %s -> physical drive %u, MBR partition %u ---", drive,
              static_cast<unsigned int>(VolToPart[volume].pd),
              static_cast<unsigned int>(VolToPart[volume].pt));

        FRESULT fr = f_mount(&g_fs[volume], drive, 1u);
        if (fr != FR_OK)
        {
            LOG_E(TAG, "%s f_mount: %s", drive, fr_name(fr));
            if (fr == FR_NO_FILESYSTEM)
                LOG_E(TAG, "%s selected partition is missing, unsupported or invalid; no formatting performed", drive);
            (void)f_mount(nullptr, drive, 0u);
            return false;
        }

        FATFS& fs = g_fs[volume];
        LOG_I(TAG, "%s mounted: %s, start LBA=%lu, cluster=%lu sectors, clusters=%lu", drive,
              fs_type_name(fs.fs_type), static_cast<unsigned long>(fs.volbase),
              static_cast<unsigned long>(fs.csize), static_cast<unsigned long>(fs.n_fatent - 2u));

        bool ok = true;
        DWORD free_clusters = 0u;
        FATFS* fsp = nullptr;
        fr = f_getfree(drive, &free_clusters, &fsp);
        if (fr != FR_OK)
        {
            LOG_E(TAG, "%s f_getfree: %s", drive, fr_name(fr));
            ok = false;
        }
        else
        {
            const uint64_t total_kib = static_cast<uint64_t>(fsp->n_fatent - 2u) * fsp->csize / 2u;
            const uint64_t free_kib = static_cast<uint64_t>(free_clusters) * fsp->csize / 2u;
            LOG_I(TAG, "%s data area: %llu KiB total, %llu KiB free", drive,
                  static_cast<unsigned long long>(total_kib), static_cast<unsigned long long>(free_kib));
            ok = list_root(root);
            if (ok)
                ok = write_then_read_back(volume);
            if (ok)
                ok = list_root(root);
        }

        fr = f_mount(nullptr, drive, 0u);
        if (fr != FR_OK)
        {
            LOG_E(TAG, "%s unmount: %s", drive, fr_name(fr));
            ok = false;
        }
        return ok;
    }
}

bool sd_fatfs_test(void)
{
    LOG_I(TAG, "FatFs R0.16: FAT12/16/32 + exFAT, UTF-8, %u explicitly mapped volumes",
          static_cast<unsigned int>(FF_VOLUMES));
    if (!report_mbr())
        return false;

    unsigned int passed = 0u;
    for (unsigned int volume = 0u; volume < FF_VOLUMES; ++volume)
    {
        const bool ok = test_volume(volume);
        LOG_I(TAG, "%u: %s", volume, ok ? "PASS" : "FAIL");
        if (ok)
            ++passed;
    }
    LOG_I(TAG, "volume tests: %u/%u passed", passed, static_cast<unsigned int>(FF_VOLUMES));
    return passed == FF_VOLUMES;
}
