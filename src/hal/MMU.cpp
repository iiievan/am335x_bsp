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
    static uint32_t s_page_table[PAGE_TABLE_NUM_ENTRY]
        __attribute__((aligned(PAGE_TABLE_ALIGN_SIZE)));

    static void init_page_table_fault_entries() noexcept
    {
        for (uint32_t idx = 0; idx < PAGE_TABLE_NUM_ENTRY; idx++)
        {
            s_page_table[idx] = 0x00;  // FAULT entry
        }
    }

    static void map_region(const Region_t& region) noexcept
    {
        uint32_t first_idx = region.startAddr >> 20;
        uint32_t base_entry = (region.startAddr & 0xFFF00000) |  // Base address (bits 31:20)
                              (region.pgType & 0x00040002) |      // Section type bits
                              region.memAttrib |
                              region.accsCtrl |
                              region.secureType;

        uint32_t* pt_entry = s_page_table + first_idx;

        for (uint32_t idx = 0; idx < region.numPages; idx++)
        {
            pt_entry[idx] = base_entry + (idx << 20);
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
        RTT_LOG_I(TAG, "Mapping DDR: 0x%08X - 0x%08X", DDR_START_ADDR, DDR_START_ADDR + (DDR_NUM_SECTIONS << 20));

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
        RTT_LOG_I(TAG, "Mapping OCMC: 0x%08X - 0x%08X", OCMC_START_ADDR, OCMC_START_ADDR + (OCMC_NUM_SECTIONS << 20));

        Region_t ocmc_region(PageType::SECTION,
                           OCMC_START_ADDR,
                           OCMC_NUM_SECTIONS,
                           MemoryType::NORMAL_NON_SHAREABLE,
                           CachePolicy::WT_NOWA,
                           CachePolicy::WB_WA,
                           SecureType::NON_SECURE,
                           AccessPermission::PRV_RW_USR_RW,
                           s_page_table);
        map_region(ocmc_region);

        // 8. Map Device region (with XN flag)
        RTT_LOG_I(TAG, "Mapping Device: 0x%08X - 0x%08X",
                  DEV_START_ADDR, DEV_START_ADDR + (DEV_NUM_SECTIONS << 20));

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
        cp15_TTB0_set((uint32_t)s_page_table);

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
            RTT_LOG_I(TAG, "PTE[0x%08X] = 0x%08X", va, s_page_table[idx]);
        else
            RTT_LOG_E(TAG, "Invalid VA for PTE dump: 0x%08X", va);
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