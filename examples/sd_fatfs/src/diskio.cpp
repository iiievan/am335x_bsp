/*  FatFs <-> AM335x MMC/SD glue.
 *
 *  Physical drive 0 is the microSD slot of the BeagleBone Black, on MMC0. FatFs calls
 *  disk_initialize() from f_mount(), so the whole controller and card bring-up happens here and the
 *  application only has to deal with the file system API.
 */

#include <cstdint>

#include "ff.h"
#include "diskio.h"

#include "log/log.h"
#include "hal/MMCSD/MMCSD.hpp"
#include "hal/MMCSD/Card.hpp"

#define TAG "sd_disk"

namespace
{
    using namespace HAL::MMCSD;

    constexpr BYTE SD_DRIVE = 0u;

    mmcsd0_t  g_mmc0;
    Card      g_card{g_mmc0};
    DSTATUS   g_status = STA_NOINIT;

    const char* status_name(const Status st) noexcept
    {
        switch (st)
        {
            case Status::OK:              return "OK";
            case Status::NOT_INITIALIZED: return "NOT_INITIALIZED";
            case Status::BAD_ARGUMENT:    return "BAD_ARGUMENT";
            case Status::TIMEOUT:         return "TIMEOUT";
            case Status::CMD_TIMEOUT:     return "CMD_TIMEOUT";
            case Status::CMD_ERROR:       return "CMD_ERROR";
            case Status::DATA_TIMEOUT:    return "DATA_TIMEOUT";
            case Status::DATA_ERROR:      return "DATA_ERROR";
            case Status::CARD_ERROR:      return "CARD_ERROR";
            default:                      return "?";
        }
    }

    const char* type_name(const CardType t) noexcept
    {
        switch (t)
        {
            case CardType::SD_V1: return "SD 1.x";
            case CardType::SD_V2: return "SD 2.0 standard capacity";
            case CardType::SD_HC: return "SDHC/SDXC";
            case CardType::MMC:   return "MMC";
            default:              return "unknown";
        }
    }
}

extern "C" DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != SD_DRIVE)
        return STA_NOINIT;

    return g_status;
}

extern "C" DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv != SD_DRIVE)
        return STA_NOINIT;

    if ((g_status & STA_NOINIT) == 0u)
        return g_status;        /* already up */

    Config cfg;
    cfg.bus_voltage  = REGS::MMCHS::VS_3_0V;
    cfg.bus_width    = REGS::MMCHS::WIDTH_1BIT;   // the card is identified on a single data line
    cfg.bus_freq_hz  = INIT_FREQ_HZ;
    cfg.data_timeout = REGS::MMCHS::DTO_TCF_2_27;

    if (!g_mmc0.init(cfg))
    {
        LOG_E(TAG, "MMC0 controller initialization failed");
        return g_status;
    }

    if (!g_mmc0.send_init_stream())
    {
        LOG_E(TAG, "the 80-clock initialization stream did not complete");
        return g_status;
    }

    if (!g_card.init(/* wide */ true, TRANSFER_FREQ_HZ))
    {
        LOG_E(TAG, "no card answered the identification sequence");
        return g_status;
    }

    char name[6];
    g_card.product_name(name);

    LOG_I(TAG, "card: %s '%s', %lu blocks (%lu MiB), 4-bit bus at %lu Hz",
          type_name(g_card.type()), name,
          static_cast<unsigned long>(g_card.block_count()),
          static_cast<unsigned long>(g_card.capacity_bytes() >> 20),
          static_cast<unsigned long>(TRANSFER_FREQ_HZ));

    g_status = 0u;

    if (g_mmc0.is_card_write_protected())
    {
        LOG_W(TAG, "the slot reports the card as write protected");
        g_status |= STA_PROTECT;
    }

    return g_status;
}

extern "C" DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
    if (pdrv != SD_DRIVE)
        return RES_PARERR;

    if ((g_status & STA_NOINIT) != 0u)
        return RES_NOTRDY;

    if ((buff == nullptr) || (count == 0u))
        return RES_PARERR;

    const Status st = g_card.read_blocks(static_cast<uint32_t>(sector),
                                         reinterpret_cast<uint8_t*>(buff),
                                         static_cast<uint32_t>(count));

    if (st != Status::OK)
    {
        LOG_E(TAG, "read of %lu block(s) at LBA %lu failed: %s",
              static_cast<unsigned long>(count),
              static_cast<unsigned long>(sector),
              status_name(st));
        return RES_ERROR;
    }

    return RES_OK;
}

#if FF_FS_READONLY == 0

extern "C" DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
    if (pdrv != SD_DRIVE)
        return RES_PARERR;

    if ((g_status & STA_NOINIT) != 0u)
        return RES_NOTRDY;

    if ((g_status & STA_PROTECT) != 0u)
        return RES_WRPRT;

    if ((buff == nullptr) || (count == 0u))
        return RES_PARERR;

    const Status st = g_card.write_blocks(static_cast<uint32_t>(sector),
                                          reinterpret_cast<const uint8_t*>(buff),
                                          static_cast<uint32_t>(count));

    if (st != Status::OK)
    {
        LOG_E(TAG, "write of %lu block(s) at LBA %lu failed: %s",
              static_cast<unsigned long>(count),
              static_cast<unsigned long>(sector),
              status_name(st));
        return RES_ERROR;
    }

    return RES_OK;
}

#endif /* FF_FS_READONLY */

extern "C" DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
    if (pdrv != SD_DRIVE)
        return RES_PARERR;

    if ((g_status & STA_NOINIT) != 0u)
        return RES_NOTRDY;

    switch (cmd)
    {
        case CTRL_SYNC:
            /*  write_blocks() only returns once the card has left the programming state, so there
             *  is never anything left in flight here. */
            return (g_card.wait_ready() == Status::OK) ? RES_OK : RES_ERROR;

        case GET_SECTOR_COUNT:
            if (buff == nullptr)
                return RES_PARERR;
            *static_cast<LBA_t*>(buff) = static_cast<LBA_t>(g_card.block_count());
            return RES_OK;

        case GET_SECTOR_SIZE:
            if (buff == nullptr)
                return RES_PARERR;
            *static_cast<WORD*>(buff) = static_cast<WORD>(BLOCK_SIZE);
            return RES_OK;

        case GET_BLOCK_SIZE:
            /*  Erase block size in sectors. The CSD carries the real value; one sector is the
             *  safe answer and is only used by f_mkfs, which this build does not enable. */
            if (buff == nullptr)
                return RES_PARERR;
            *static_cast<DWORD*>(buff) = 1u;
            return RES_OK;

        default:
            return RES_PARERR;
    }
}
