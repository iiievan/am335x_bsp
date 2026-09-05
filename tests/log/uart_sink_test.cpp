#include "log/sinks/UartSink.hpp"
#include "log/sinks/RttSink.hpp"
#include "SEGGER_RTT.h"
#include <cassert>
#include <cstdio>
#include <string>
#include <type_traits>

static std::string rtt_output;
static unsigned time_reads;
extern "C" uint64_t log_get_system_time_ms(void)
{
    ++time_reads;
    return 12345u;
}
extern "C" void SEGGER_RTT_Init(void) { }
extern "C" unsigned SEGGER_RTT_Write(unsigned, const void* data, unsigned size)
{
    rtt_output.append(static_cast<const char*>(data), size);
    return size;
}

struct FakeUart
{
    enum class Mode { UNINITIALIZED, POLLING, INTERRUPT, DMA };
    Mode mode{Mode::UNINITIALIZED};
    std::string output;
    bool fail{false};
    unsigned writes{0u};
    uint32_t budget{0u};
    Mode io_mode() const { return mode; }
    bool put_data_bounded(const void* data, std::size_t size, uint32_t loops)
    {
        ++writes;
        budget = loops;
        if (fail)
            return false;
        output.append(static_cast<const char*>(data), size);
        return true;
    }
};

static void discard(void*, log_level_t, const char*, std::size_t) { }

int main()
{
    using Sink = HAL::LOG::UartSink<FakeUart>;
    static_assert(!std::is_copy_constructible_v<Sink>);
    static_assert(!std::is_move_constructible_v<Sink>);
    FakeUart uart;
    HAL::LOG::RttSink rtt;
    assert(rtt.registered());
    {
        Sink sink{uart, 123u};
        assert(sink.registered());
        LOG_I("TEST", "early");
        assert(uart.writes == 0u && sink.dropped() == 1u);
        uart.mode = FakeUart::Mode::POLLING;
        rtt_output.clear();
        time_reads = 0u;
        LOG_I("TEST", "value=%u", 42u);
        assert(time_reads == 1u);
        assert(uart.output == "[   12.345]:\t[I][TEST]:\tvalue=42\r\n");
        assert(rtt_output == "\033[0;32m[   12.345]:\t[I][TEST]:\tvalue=42\n\033[0m");
        assert(uart.budget == 123u && sink.delivered() == 1u);
        uart.output.clear();
        LOG_I("TEST", "one\r\ntwo\nthree");
        assert(uart.output == "[   12.345]:\t[I][TEST]:\tone\r\ntwo\r\nthree\r\n");
        sink.enable(false);
        auto writes = uart.writes;
        LOG_I("TEST", "disabled");
        assert(uart.writes == writes);
        sink.enable();
        for (auto mode : {FakeUart::Mode::DMA, FakeUart::Mode::INTERRUPT,
                          FakeUart::Mode::UNINITIALIZED})
        {
            uart.mode = mode;
            LOG_I("TEST", "busy or not initialized");
            assert(uart.writes == writes);
        }
        uart.mode = FakeUart::Mode::POLLING;
        uart.fail = true;
        auto dropped = sink.dropped();
        LOG_I("TEST", "timeout");
        assert(sink.dropped() == dropped + 1u);
        assert(uart.writes == writes + 1u);
        uart.fail = false;
        LOG_I("TEST", "recovered");
        assert(sink.delivered() == 3u);
        assert(log_exclude_tag("TEST"));
        writes = uart.writes;
        LOG_I("TEST", "filtered");
        assert(uart.writes == writes);
        log_unexclude_tag("TEST");
    }
    const auto writes = uart.writes;
    LOG_I("TEST", "after destruction");
    assert(uart.writes == writes);
    // With RTT taking one slot, fill the three remaining slots.
    log_sink_t slots[3]{{nullptr, discard}, {nullptr, discard}, {nullptr, discard}};
    for (auto& slot : slots)
        assert(log_register_sink(&slot));
    {
        Sink rejected{uart};
        assert(!rejected.registered());
    }
    for (auto& slot : slots)
        log_unregister_sink(&slot);
    { Sink again{uart}; assert(again.registered()); }
    std::puts("PASS: fan-out, timestamp once, CRLF, inactive modes, enable/disable,");
    std::puts("      timeout/recovery, TAG filter, RAII lifetime, registry capacity");
}
