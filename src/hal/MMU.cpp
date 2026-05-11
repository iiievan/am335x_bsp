/*=======================================================================*/
/*  hal_mmu_cache.cpp - MMU & Cache implementation                      */
/*=======================================================================*/
#include "hal/MMU.hpp"
#include "hal/CACHE.hpp"
#include "hal/BRANCH_PREDICTION.hpp"
#include "startup/cp15.h"
#include "rtt/rtt_log.h"

#define TAG "MMU"

namespace HAL::MMU
{
    constexpr uint32_t MMU_PAGETABLE_ENTRY_FAULT   = 0x00;
    constexpr uint32_t MMU_PAGEBOUND_SHIFT         = 20;
    constexpr uint32_t MMU_PG_SUPSECT_SIZE_SHIFT   = 14;
    constexpr uint32_t MMU_PGADDR_MASK             = 0xFFF00000;
    constexpr uint32_t MMU_PGTYPE_MASK             = 0x00040002;

    static uint32_t s_page_table[PAGE_TABLE_NUM_ENTRY]  __attribute__((aligned(PAGE_TABLE_ALIGN_SIZE)));

    static void init_page_table_fault_entries() noexcept
    {
        for (uint32_t idx = 0; idx < PAGE_TABLE_NUM_ENTRY; idx++)
        {
            s_page_table[idx] = MMU_PAGETABLE_ENTRY_FAULT;
        }
    }

    static void map_region(const Region_t& region) noexcept
    {
        uint32_t *ptEntryPtr;
        uint32_t ptEntry;

        /* Get the first entry in the page table to set */
        ptEntryPtr = region.masterPtPtr + (region.startAddr >> MMU_PAGEBOUND_SHIFT);

        /* Set the pointer to the last entry */
        ptEntryPtr += (region.numPages - 1);

        /* Get the start Address MSB 3 nibbles. Ignore extended address */
        ptEntry = (region.startAddr & region.pgType) & MMU_PGADDR_MASK;

        /*
        ** Update the page table entry with memory attributes and
        ** Access Permissions and Security.
        ** All the regions will be marked as global.
        */
        ptEntry |= ((MMU_PGTYPE_MASK & region.pgType)
                    | region.accsCtrl | region.memAttrib
                    | region.secureType);

        /* Set the entries in the page table for the region attributes */
        for(int idx = (region.numPages - 1); idx >= 0; idx--)
        {
            *ptEntryPtr-- = ptEntry + (idx << MMU_PAGEBOUND_SHIFT) ;
        }
    }

    void init() noexcept
    {
        RTT_LOG_I(TAG, "Initializing MMU page tables...");

        // 1. Invalidate TLB
        cp15_TLB_invalidate();

        // 2. Set domain access to client mode
        cp15_domain_access_client_set();

        // 3. Disable TEX remap, Access Flag, Alignment check, and MMU
        cp15_control_feature_disable(CP15_CONTROL_TEXREMAP |
                                            CP15_CONTROL_ACCESSFLAG |
                                            CP15_CONTROL_ALIGN_CHCK |
                                            CP15_CONTROL_MMU);

        // 4. Configure TTB Control (use only TTB0)
        cp15_TTB_ctl_TTB0_config();

        // 5. Initialize page table with fault entries
        init_page_table_fault_entries();

        // 6. Map DDR region
        RTT_LOG_I(TAG, "Mapping DDR: 0x%08X - 0x%08X", (int)DDR_START_ADDR, (int)(DDR_START_ADDR + (DDR_NUM_SECTIONS << 20)));

        Region_t ddr_region(PageType::SECTION,
                                    DDR_START_ADDR,
                              DDR_NUM_SECTIONS,
                            MemoryType::NORMAL_NON_SHAREABLE,
                            CachePolicy::WT_NOWA,
                            CachePolicy::WB_WA,
                            SecureType::NON_SECURE,
                            AccessPermission::PRV_RW_USR_RW,
                            s_page_table);
        map_region(ddr_region);

        // 7. Map OCMC region
        RTT_LOG_I(TAG, "Mapping OCMC: 0x%08X - 0x%08X", (int)OCMC_START_ADDR, (int)(OCMC_START_ADDR + (OCMC_NUM_SECTIONS << 20)));

        Region_t ocmc_region(PageType::SECTION,
                           OCMC_START_ADDR,
                           OCMC_NUM_SECTIONS,
                           MemoryType::NORMAL_NON_SHAREABLE,
                           CachePolicy::NON_CACHEABLE, // For RTT working
                           CachePolicy::NON_CACHEABLE,
                           SecureType::NON_SECURE,
                           AccessPermission::PRV_RW_USR_RW,
                           s_page_table);
        map_region(ocmc_region);

        // 8. Map Device region (with XN flag)
        RTT_LOG_I(TAG, "Mapping Device: 0x%08X - 0x%08X",
                  (int)DEV_START_ADDR, (int)(DEV_START_ADDR + (DEV_NUM_SECTIONS << 20)));

        Region_t dev_region(PageType::SECTION,
                          DEV_START_ADDR,
                          DEV_NUM_SECTIONS,
                          MemoryType::DEVICE_SHAREABLE,
                          SecureType::SECURE,
                          AccessPermission::PRV_RW_USR_RW | SectionFlag::EXEC_NEVER,
                          s_page_table);
        map_region(dev_region);

        RTT_LOG_I(TAG, "MMU page tables initialized");
    }

    void enable() noexcept
    {
        RTT_LOG_I(TAG, "Enabling MMU...");

        // Set TTB0 register
        cp15_TTB0_set(reinterpret_cast<uint32_t>(s_page_table));

        // Enable MMU
        cp15_MMU_enable();

        // Synchronization barrier
        cp15_DSB_ISB_sync_barrier();

        RTT_LOG_I(TAG, "MMU enabled");
    }

    void disable() noexcept
    {
        RTT_LOG_I(TAG, "Disabling MMU...");
        cp15_MMU_disable();
        cp15_DSB_ISB_sync_barrier();
        RTT_LOG_I(TAG, "MMU disabled");
    }

    bool is_enabled() noexcept
    {
        uint32_t sctlr;
        __asm volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r" (sctlr));
        return (sctlr & 0x1) != 0;
    }

    uint32_t* get_page_table() noexcept
    {
        return s_page_table;
    }

    void dump_page_table_entry(uint32_t va) noexcept
    {
        uint32_t idx = va >> 20;
        if (idx < PAGE_TABLE_NUM_ENTRY)
            RTT_LOG_I(TAG, "PTE[0x%08X] = 0x%08X", (int)va, (int)s_page_table[idx]);
        else
            RTT_LOG_E(TAG, "Invalid VA for PTE dump: 0x%08X", (int)va);
    }
}

extern "C"
{
    void init_memory(void) noexcept
    {
        RTT_LOG_I(TAG, "=== InitMem: MMU & Cache initialization ===");

        HAL::MMU::init();

        HAL::MMU::enable();

        HAL::CACHE::init();
        HAL::CACHE::enable(HAL::CACHE::Type::ALL);

        HAL::BRANCH_PREDICTION::invalidate();
        HAL::BRANCH_PREDICTION::enable();

        RTT_LOG_I(TAG, "=== InitMem complete ===");
        RTT_LOG_I(TAG, "MMU: %s, I-Cache: %s, D-Cache: %s, Branch Pred: %s",
                  HAL::MMU::is_enabled() ? "ON" : "OFF",
                  HAL::CACHE::is_icache_enabled() ? "ON" : "OFF",
                  HAL::CACHE::is_dcache_enabled() ? "ON" : "OFF",
                  HAL::BRANCH_PREDICTION::is_enabled() ? "ON" : "OFF");
    }
} // extern "C"