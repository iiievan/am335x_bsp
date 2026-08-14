/*=======================================================================*/
/*  hal/PERF.cpp - ARM Cortex-A8 PMU Implementation                     */
/*=======================================================================*/
#include "hal/PERF.hpp"
#include "regs/PRCM.hpp"
#include "startup/cp15.h"
#include "rtt/rtt_log.h"
#include <cstring>

#define TAG "PERF"

namespace HAL::PERF
{
    void init() noexcept
    {
        // Grant the user/supervisor access to the PMU in CP15
        // PMUSERENR (c9, c14, 0) -> Bit 0 = EN
        uint32_t user_en = 1;
        __asm volatile("mcr p15, 0, %0, c9, c14, 0" :: "r"(user_en));

        // Resetting and Configuring the PMCR (Performance Monitor Control Register)
        // Bit 0: E (Enable all counters)
        // Bit 1: P (Event counter reset)
        // Bit 2: C (Cycle counter reset)
        // Bit 3: CLKDIV (0 = 1 cycle per clock, 1 = 1 cycle per 64 clocks)
        uint32_t pmcr = (1 << 0) | (1 << 1) | (1 << 2);
        __asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(pmcr));

        // Enable the cycle counter (PMCCNTR) using PMCNTENSET (c9, c12, 1)
        uint32_t pmcntenset = (1U << 31);
        __asm volatile("mcr p15, 0, %0, c9, c12, 1" :: "r"(pmcntenset));

        RTT_LOG_I(TAG, "Cortex-A8 PMU Hardware initialized");
    }

    void reset_all() noexcept
    {
        uint32_t pmcr;
        __asm volatile("mrc p15, 0, %0, c9, c12, 0" : "=r"(pmcr));
        pmcr |= (1 << 1) | (1 << 2); // Reset the event counters and the cycle counter
        __asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(pmcr));
    }

    void enable() noexcept
    {
        uint32_t pmcr;
        __asm volatile("mrc p15, 0, %0, c9, c12, 0" : "=r"(pmcr));
        pmcr |= (1 << 0);
        __asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(pmcr));
    }

    void disable() noexcept
    {
        uint32_t pmcr;
        __asm volatile("mrc p15, 0, %0, c9, c12, 0" : "=r"(pmcr));
        pmcr &= ~(1 << 0);
        __asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(pmcr));
    }

    void reset_cycle_count() noexcept
    {
        uint32_t pmcr;
        __asm volatile("mrc p15, 0, %0, c9, c12, 0" : "=r"(pmcr));
        pmcr |= (1 << 2);
        __asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(pmcr));
    }

    void configure_event(Counter counter, EventType event) noexcept
    {
        auto cnt_idx = static_cast<uint8_t>(counter);
        auto evt_idx = static_cast<uint8_t>(event);

        //  Selecting a counter via PMSELR (c9, c12, 5)
        __asm volatile("mcr p15, 0, %0, c9, c12, 5" :: "r"(cnt_idx));

        // Store the event type in PMXEVTYPER (c9, c13, 1)
        __asm volatile("mcr p15, 0, %0, c9, c13, 1" :: "r"(evt_idx));

        // Enable the selected counter using PMCNTENSET (c9, c12, 1)
        uint32_t mask = (1U << cnt_idx);
        __asm volatile("mcr p15, 0, %0, c9, c12, 1" :: "r"(mask));
    }

    uint32_t read_event(Counter counter) noexcept
    {
        auto cnt_idx = static_cast<uint8_t>(counter);
        // Select counter
        __asm volatile("mcr p15, 0, %0, c9, c12, 5" :: "r"(cnt_idx));

        // Read a value from PMXEVCNTR (c9, c13, 2)
        uint32_t val;
        __asm volatile("mrc p15, 0, %0, c9, c13, 2" : "=r"(val));
        return val;
    }

    void reset_event(Counter counter) noexcept
    {
        auto cnt_idx = static_cast<uint8_t>(counter);
        __asm volatile("mcr p15, 0, %0, c9, c12, 5" :: "r"(cnt_idx));
        uint32_t zero = 0;
        __asm volatile("mcr p15, 0, %0, c9, c13, 2" :: "r"(zero));
    }

    void run_ddr_benchmark() noexcept
    {
        constexpr size_t TEST_SIZE_BYTES = 1 * 1024 * 1024; // 1 МБ
        constexpr size_t TEST_WORDS = TEST_SIZE_BYTES / sizeof(uint32_t);

        // Highlighting buffers in the DDR region
        static uint32_t src_buf[TEST_WORDS] __attribute__((section(".perf_data")));
        static uint32_t dst_buf[TEST_WORDS] __attribute__((section(".perf_data")));

        RTT_LOG_I(TAG, "=== Running Memory Performance Benchmark ===");

        for (size_t i = 0; i < TEST_WORDS; ++i)
        {
            src_buf[i] = static_cast<uint32_t>(i);
        }

        // Configuring event counters for D-Cache and L2 Cache misses
        configure_event(Counter::COUNTER_0, EventType::L1D_CACHE_REFILL);
        configure_event(Counter::COUNTER_1, EventType::L2D_CACHE_REFILL);

        reset_all();

        const uint32_t t_start = get_cycle_count();
        const uint32_t l1_miss_start = read_event(Counter::COUNTER_0);
        const uint32_t l2_miss_start = read_event(Counter::COUNTER_1);

        //  Copying 1 MB of memory (32-bit)
        std::memcpy(dst_buf, src_buf, TEST_SIZE_BYTES);

        const uint32_t t_end = get_cycle_count();
        const uint32_t l1_miss_end = read_event(Counter::COUNTER_0);
        const uint32_t l2_miss_end = read_event(Counter::COUNTER_1);

        const uint32_t elapsed_cycles = t_end - t_start;
        const uint32_t l1_misses = l1_miss_end - l1_miss_start;
        const uint32_t l2_misses = l2_miss_end - l2_miss_start;

        const uint32_t cpu_freq = get_mpu_freq_hz();
        const float seconds = static_cast<float>(elapsed_cycles) / static_cast<float>(cpu_freq);
        const float mb_per_sec = (1.0f / seconds);
        const uint32_t elapsed_ms = ticks_to_ms(elapsed_cycles, cpu_freq);
        RTT_LOG_I(TAG, "Memcpy 1MB Result:");
        RTT_LOG_I(TAG, "  Elapsed CPU Cycles/ms : %u/%u", (unsigned)elapsed_cycles,(unsigned)elapsed_ms);
        RTT_LOG_I(TAG, "  Calculated Speed   : %u MB/s (at %u MHz)", (unsigned)mb_per_sec,(unsigned)(static_cast<float>(cpu_freq)/1000000.0f));
        RTT_LOG_I(TAG, "  L1 D-Cache Refills : %u", (unsigned)l1_misses);
        RTT_LOG_I(TAG, "  L2 Cache Refills   : %u", (unsigned)l2_misses);
        RTT_LOG_I(TAG, "===========================================");
    }

    uint32_t get_mpu_freq_hz()
    {
        using namespace REGS::PRCM;
        const auto& wkup = *AM335x_CM_WKUP; //

        // Extract N (DPLL_DIV) and M (DPLL_MULT) from CLKSEL_DPLL_MPU
        const uint32_t clksel = wkup.CLKSEL_DPLL_MPU.reg;
        const uint32_t mult = (clksel >> 8) & 0x7FF; // Bits 8..18 (DPLL_MULT)
        const uint32_t div  = (clksel & 0x7F) + 1;   // Bits 0..6 (DPLL_DIV, делитель = N + 1)

        // Extract post-divider M2 from DIV_M2_DPLL_MPU
        uint32_t m2 = wkup.DIV_M2_DPLL_MPU.b.DPLL_CLKOUT_DIV;
        if (m2 == 0) {
            m2 = 1; //  Protection set to 0; a value of 0/1 in the TRM corresponds to a divisor of 1
        }

        constexpr uint32_t OSC_FREQ_HZ = 24000000U; // 24 MHz

        // Calculation: (24MHz * M) / (N+1) / M2
        const uint64_t freq = (static_cast<uint64_t>(OSC_FREQ_HZ) * mult) / (div * m2);
        return static_cast<uint32_t>(freq);
    }

    ScopedProfiler::ScopedProfiler(const char* tag) noexcept
        : m_tag(tag)
    {
        m_start_cycles = get_cycle_count();
        m_start_evt0   = read_event(Counter::COUNTER_0);
        m_start_evt1   = read_event(Counter::COUNTER_1);
    }

    ScopedProfiler::~ScopedProfiler()
    {
        const uint32_t cycles = get_cycle_count() - m_start_cycles;
        const uint32_t evt0   = read_event(Counter::COUNTER_0) - m_start_evt0;
        const uint32_t evt1   = read_event(Counter::COUNTER_1) - m_start_evt1;
        const uint32_t cpu_freq = get_mpu_freq_hz();
        const uint32_t elapsed_us = ticks_to_us(cycles, cpu_freq);
        const uint32_t elapsed_ms = ticks_to_ms(cycles, cpu_freq);

        RTT_LOG_I(m_tag, "Prof: %u cycles | %u us | %u ms | Evt0: %u | Evt1: %u", (unsigned)cycles,
                                                                                  (unsigned)elapsed_us,
                                                                                  (unsigned)elapsed_ms,
                                                                                  (unsigned)evt0,
                                                                                  (unsigned)evt1);
    }
}