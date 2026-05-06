/*=======================================================================*/
/*  hal_mmu_cache.hpp - MMU & Cache initialization for AM335x           */
/*=======================================================================*/
#ifndef __MMU_HPP
#define __MMU_HPP

#include <cstdint>

namespace HAL::MMU
{
    constexpr uint32_t PAGE_TABLE_NUM_ENTRY   = 4096;
    constexpr uint32_t PAGE_TABLE_ALIGN_SIZE  = 16 * 1024; // 16384 - 16KB alignment

    constexpr uint32_t DDR_START_ADDR   = 0x80000000;
    constexpr uint32_t DDR_NUM_SECTIONS = 128;          // 128MB

    constexpr uint32_t OCMC_START_ADDR  = 0x40300000;
    constexpr uint32_t OCMC_NUM_SECTIONS = 1;           // 1MB

    constexpr uint32_t DEV_START_ADDR   = 0x44000000;
    constexpr uint32_t DEV_NUM_SECTIONS = 960;          // 960MB

    enum class PageType : uint32_t {
        SECTION      = 0xFFF04002,   // 1MB section
        SUPERSECTION = 0xFF040002    // 16MB supersection
    };

    enum class MemoryType : uint32_t
    {
        STRONGLY_ORDERED_SHAREABLE = 0x00000000,
        DEVICE_SHAREABLE           = 0x00000004,
        DEVICE_NON_SHAREABLE       = 0x00002000,
        NORMAL_NON_SHAREABLE       = 0x00004000,
        NORMAL_SHAREABLE           = 0x00014000
    };

    enum class CachePolicy : uint32_t
    {
        NON_CACHEABLE   = 0x00,
        WB_WA           = 0x01,   // Write Back, Write Allocate
        WT_NOWA         = 0x02,   // Write Through, No Write Allocate
        WB_NOWA         = 0x03    // Write Back, No Write Allocate
    };

    enum class SecureType : uint32_t
    {
        SECURE    = 0x00000000,
        NON_SECURE = 0x00080000
    };

    enum class AccessPermission : uint32_t
    {
        PRV_NA_USR_NA = 0x0000,   // No access in both modes
        PRV_RW_USR_NA = 0x0400,   // R/W privileged, no access user
        PRV_RW_USR_RO = 0x0800,   // R/W privileged, read only user
        PRV_RW_USR_RW = 0x0C00,   // R/W both modes
        PRV_RO_USR_NA = 0x8400    // Read only privileged, no access user
    };

    enum class SectionFlag : uint32_t
    {
        NONE       = 0x00000000,
        EXEC_NEVER = 0x00000010   // XN bit for read-sensitive sections
    };

    inline SectionFlag operator|(SectionFlag a, SectionFlag b)              { return static_cast<SectionFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }
    inline AccessPermission operator|(AccessPermission a, SectionFlag b)    { return static_cast<AccessPermission>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }

    struct Region_t
    {
        uint32_t        pgType;      // Page type
        uint32_t        startAddr;   // Start address (VA = PA)
        uint32_t        numPages;    // Number of 1MB pages
        uint32_t        memAttrib;   // Memory type and cache settings
        uint32_t        secureType;  // Secure/Non-secure
        uint32_t        accsCtrl;    // Access permissions
        uint32_t*       masterPtPtr; // Pointer to master page table

        // For Normal memory (with cache policies)
        Region_t(PageType type,
               uint32_t start,
               uint32_t pages,
               MemoryType memType,
               CachePolicy inner,
               CachePolicy outer,
               SecureType secure,
               AccessPermission access,
               uint32_t* pageTable)
            : pgType(static_cast<uint32_t>(type))
            , startAddr(start)
            , numPages(pages)
            , memAttrib(static_cast<uint32_t>(memType) |
                        (static_cast<uint32_t>(outer) << 12) |
                        (static_cast<uint32_t>(inner) << 2))
            , secureType(static_cast<uint32_t>(secure))
            , accsCtrl(static_cast<uint32_t>(access))
            , masterPtPtr(pageTable)
        {}

        // For Device memory
        Region_t(PageType type,
               uint32_t start,
               uint32_t pages,
               MemoryType memType,
               SecureType secure,
               AccessPermission access,
               uint32_t* pageTable)
            : pgType(static_cast<uint32_t>(type))
            , startAddr(start)
            , numPages(pages)
            , memAttrib(static_cast<uint32_t>(memType))
            , secureType(static_cast<uint32_t>(secure))
            , accsCtrl(static_cast<uint32_t>(access))
            , masterPtPtr(pageTable)
        {}

        // For memAttrib (for complex cases)
        Region_t(PageType type,
               uint32_t start,
               uint32_t pages,
               uint32_t memAttrib,
               SecureType secure,
               AccessPermission access,
               uint32_t* pageTable)
            : pgType(static_cast<uint32_t>(type))
            , startAddr(start)
            , numPages(pages)
            , memAttrib(memAttrib)
            , secureType(static_cast<uint32_t>(secure))
            , accsCtrl(static_cast<uint32_t>(access))
            , masterPtPtr(pageTable)
        {}
    };

    void init() noexcept;
    void enable() noexcept;
    void disable() noexcept;
    bool is_enabled() noexcept;

    uint32_t* get_page_table() noexcept;
    void dump_page_table_entry(uint32_t va) noexcept;
}

extern "C"
{
    void init_memory(void) noexcept;
}

#endif // __MMU_HPP