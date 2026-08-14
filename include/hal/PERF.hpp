/*=======================================================================*/
/*  hal/PERF.hpp - ARM Cortex-A8 Performance Monitor Unit (PMU) HAL      */
/*=======================================================================*/
#ifndef __HAL_PERF_HPP
#define __HAL_PERF_HPP

#include <cstdint>
#include <cstddef>

namespace HAL::PERF
{
    // События PMU для ARM Cortex-A8 (ARMv7-A)
    enum class EventType : uint8_t
    {
        SW_INCREMENT            = 0x00,
        L1I_CACHE_REFILL        = 0x01, // Промах L1 Instruction Cache
        L1I_TLB_REFILL          = 0x02, // Промах Instruction TLB
        L1D_CACHE_REFILL        = 0x03, // Промах L1 Data Cache
        L1D_CACHE_ACCESS        = 0x04, // Обращение к L1 Data Cache
        L1D_TLB_REFILL          = 0x05, // Промах Data TLB
        INSTR_EXECUTED          = 0x08, // Выполненные инструкции
        EXC_TAKEN               = 0x09, // Исключения/Прерывания
        BRANCH_MISPRED          = 0x10, // Сбой предсказания перехода
        CPU_CYCLES              = 0x11, // Такты CPU
        BRANCH_PRED             = 0x12, // Все переходы (предсказанные)
        UNALIGNED_ACCESS        = 0x13, // Невыровненные обращения
        L2D_CACHE_REFILL        = 0x43, // Промах L2 Cache (Cortex-A8)
        L2D_CACHE_ACCESS        = 0x44  // Доступ к L2 Cache (Cortex-A8)
    };

    enum class Counter : uint8_t
    {
        COUNTER_0 = 0,
        COUNTER_1 = 1,
        COUNTER_2 = 2,
        COUNTER_3 = 3
    };

    struct PerformanceMetrics
    {
        uint32_t cycles;
        uint32_t counter0_evt;
        uint32_t counter0_val;
        uint32_t counter1_evt;
        uint32_t counter1_val;
    };

    // Инициализация и глобальный контроль PMU
    void init() noexcept;
    void reset_all() noexcept;
    void enable() noexcept;
    void disable() noexcept;

    // Счётчик тактов (PMCCNTR)
    inline uint32_t get_cycle_count() noexcept
    {
        uint32_t val;
        __asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(val));
        return val;
    }

    void reset_cycle_count() noexcept;

    // Конфигурация программируемых счётчиков событий (Event Counters 0..3)
    void configure_event(Counter counter, EventType event) noexcept;
    uint32_t read_event(Counter counter) noexcept;
    void reset_event(Counter counter) noexcept;

    // Синтетический бенчмарк памяти DDR
    void run_ddr_benchmark() noexcept;

    uint32_t get_mpu_freq_hz();
    inline uint32_t ticks_to_ms(const uint64_t ticks, const uint32_t cpu_freq_hz)
    {
        return static_cast<uint32_t>(ticks / (cpu_freq_hz / 1000U));
    }

    inline uint32_t ticks_to_us(const uint64_t ticks, const uint32_t cpu_freq_hz)
    {
        return static_cast<uint32_t>(ticks / (cpu_freq_hz / 1000000U));
    }

    // RAII-профайлер для замера участка кода по тактам и событиям
    class ScopedProfiler
    {
    public:
        explicit ScopedProfiler(const char* tag) noexcept;
        ~ScopedProfiler();

    private:
        const char* m_tag;
        uint32_t m_start_cycles;
        uint32_t m_start_evt0;
        uint32_t m_start_evt1;
    };
}

#endif // __HAL_PERF_HPP