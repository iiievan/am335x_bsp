#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "init.h"
#include "hal/UartDma.hpp"
#include "hal/PERF.hpp"
#include "hal/boards/beaglebone_black.hpp"
#include "line_editor.hpp"
#include "rtt/rtt_log.h"

#define TAG "uart_cli"

namespace
{
    using Uart = HAL::UART::uart0_t;

    constexpr std::size_t TEST_DATA_SIZE = 8u;
    constexpr uint32_t TEST_TIMEOUT_LOOPS = 500'000'000u;

    constexpr uint32_t DMA_FRAME_MAGIC = 0x55415254u; // "TRAU" in LE memory
    constexpr uint16_t DMA_FRAME_VERSION = 1u;
    constexpr std::size_t DMA_FRAME_HEADER_SIZE = 16u;
    constexpr std::size_t DMA_FRAME_CRC_SIZE = 2u;
    constexpr std::size_t DMA_ALIGNMENT = 8u;
    constexpr std::size_t DMA_MAX_PAYLOAD_SIZE = 6144u;
    constexpr std::size_t DMA_MAX_FRAME_SIZE =
        DMA_FRAME_HEADER_SIZE + DMA_MAX_PAYLOAD_SIZE + 6u + DMA_FRAME_CRC_SIZE;

    constexpr char POLLING_PATTERN[] = "POLL1234";
    constexpr char INTERRUPT_PATTERN[] = "IRQ12345";
    constexpr char DMA_PATTERN[] = "DMA12345";

    constexpr const char* COMMAND_NAMES[] =
    {
        "help",
        "test polling",
        "test interrupt",
        "test dma",
        "test all",
        "auto dma"
    };
    constexpr std::size_t COMMAND_COUNT =
        sizeof(COMMAND_NAMES) / sizeof(COMMAND_NAMES[0]);

    volatile char g_interrupt_buffer[TEST_DATA_SIZE]{};
    volatile std::size_t g_interrupt_count{0u};
    volatile bool g_interrupt_complete{false};
    volatile bool g_interrupt_active{false};
    alignas(64) uint8_t g_dma_frame[DMA_MAX_FRAME_SIZE]{};

    void log_dma_hw_state(const char* phase, const uint8_t channel) noexcept
    {
        using namespace REGS::EDMA;
        const auto region = HAL::EDMA::get_region_id();
        const auto& cc = *AM335X_EDMA3CC;
        const auto& uart = *REGS::UART::AM335X_UART_0;
        const auto* words = reinterpret_cast<const volatile uint32_t*>(&cc.paRAM(channel));

        RTT_LOG_I("dma_auto",
                  "%s ch=%u UART LSR=%08x SSR=%08x SCR=%08x RXFIFO=%u TXFIFO=%u",
                  phase, static_cast<unsigned>(channel),
                  static_cast<unsigned>(uart.LSR_UART.reg),
                  static_cast<unsigned>(uart.SSR.reg),
                  static_cast<unsigned>(uart.SCR.reg),
                  static_cast<unsigned>(uart.RXFIFO_LVL.b.RXFIFO_LVL),
                  static_cast<unsigned>(uart.TXFIFO_LVL.b.TXFIFO_LVL));
        RTT_LOG_I("dma_auto",
                  "%s ER=%08x EER=%08x SER=%08x IPR=%08x EMR=%08x CCERR=%08x DRAE=%08x",
                  phase,
                  static_cast<unsigned>(cc.S_ER(region).reg),
                  static_cast<unsigned>(cc.S_EER(region).reg),
                  static_cast<unsigned>(cc.S_SER(region).reg),
                  static_cast<unsigned>(cc.S_IPR(region).reg),
                  static_cast<unsigned>(cc.EMR.reg),
                  static_cast<unsigned>(cc.CCERR.reg),
                  static_cast<unsigned>(cc.DRAE(region).reg));
        RTT_LOG_I("dma_auto", "%s PaRAM[%u] %08x %08x %08x %08x",
                  phase, static_cast<unsigned>(channel),
                  static_cast<unsigned>(words[0]), static_cast<unsigned>(words[1]),
                  static_cast<unsigned>(words[2]), static_cast<unsigned>(words[3]));
        RTT_LOG_I("dma_auto", "%s PaRAM[%u] %08x %08x %08x %08x",
                  phase, static_cast<unsigned>(channel),
                  static_cast<unsigned>(words[4]), static_cast<unsigned>(words[5]),
                  static_cast<unsigned>(words[6]), static_cast<unsigned>(words[7]));
    }

    class PollingRestore final
    {
    public:
        explicit PollingRestore(Uart& uart) noexcept : m_uart(uart) {}
        ~PollingRestore() noexcept { m_uart.init_polling(); }

        PollingRestore(const PollingRestore&) = delete;
        PollingRestore& operator=(const PollingRestore&) = delete;

    private:
        Uart& m_uart;
    };

    struct DmaFrameHeader
    {
        uint32_t magic;
        uint16_t version;
        uint16_t sequence;
        uint32_t payload_size;
        uint32_t seed;
    };
    static_assert(sizeof(DmaFrameHeader) == DMA_FRAME_HEADER_SIZE);

    [[nodiscard]] constexpr std::size_t dma_frame_size(const std::size_t payload_size) noexcept
    {
        const std::size_t without_padding =
            DMA_FRAME_HEADER_SIZE + payload_size + DMA_FRAME_CRC_SIZE;
        return (without_padding + DMA_ALIGNMENT - 1u) & ~(DMA_ALIGNMENT - 1u);
    }

    [[nodiscard]] uint16_t crc16_ccitt_false(const uint8_t* data,
                                             const std::size_t size) noexcept
    {
        uint16_t crc = 0xFFFFu;
        for (std::size_t i = 0u; i < size; ++i)
        {
            crc ^= static_cast<uint16_t>(data[i]) << 8u;
            for (uint8_t bit = 0u; bit < 8u; ++bit)
                crc = (crc & 0x8000u) != 0u
                          ? static_cast<uint16_t>((crc << 1u) ^ 0x1021u)
                          : static_cast<uint16_t>(crc << 1u);
        }
        return crc;
    }

    [[nodiscard]] uint32_t xorshift32(uint32_t& state) noexcept
    {
        state ^= state << 13u;
        state ^= state >> 17u;
        state ^= state << 5u;
        return state;
    }

    [[nodiscard]] bool parse_u32(const char*& input, uint32_t& value) noexcept
    {
        while (*input == ' ')
            ++input;
        if (*input < '0' || *input > '9')
            return false;

        uint32_t result = 0u;
        do
        {
            const uint32_t digit = static_cast<uint32_t>(*input - '0');
            if (result > (0xFFFFFFFFu - digit) / 10u)
                return false;
            result = result * 10u + digit;
            ++input;
        } while (*input >= '0' && *input <= '9');

        value = result;
        return true;
    }

    [[nodiscard]] bool parse_auto_dma(const char* command,
                                      uint32_t& payload_size,
                                      uint32_t& sequence,
                                      uint32_t& seed) noexcept
    {
        constexpr char prefix[] = "auto dma";
        for (std::size_t i = 0u; i < sizeof(prefix) - 1u; ++i)
        {
            if (command[i] != prefix[i])
                return false;
        }

        const char* cursor = command + sizeof(prefix) - 1u;
        if (!parse_u32(cursor, payload_size) ||
            !parse_u32(cursor, sequence) ||
            !parse_u32(cursor, seed))
        {
            return false;
        }
        while (*cursor == ' ')
            ++cursor;
        return *cursor == '\0';
    }

    [[nodiscard]] bool validate_dma_payload(const DmaFrameHeader& header) noexcept
    {
        uint32_t state = header.seed != 0u ? header.seed : 0x6D2B79F5u;
        const auto* payload = g_dma_frame + DMA_FRAME_HEADER_SIZE;
        for (std::size_t i = 0u; i < header.payload_size; ++i)
        {
            if (payload[i] != static_cast<uint8_t>(xorshift32(state) & 0xFFu))
                return false;
        }
        return true;
    }

    void run_dma_auto_test(Uart& uart,
                           const uint32_t payload_size,
                           const uint16_t sequence,
                           const uint32_t seed) noexcept
    {
        PollingRestore restore{uart};
        const std::size_t frame_size = dma_frame_size(payload_size);
        char message[192]{};

        RTT_LOG_I("dma_auto", "BEGIN seq=%u payload=%u frame=%u seed=%08x",
                  static_cast<unsigned>(sequence),
                  static_cast<unsigned>(payload_size),
                  static_cast<unsigned>(frame_size),
                  static_cast<unsigned>(seed));

        if (!uart.init_dma())
        {
            RTT_LOG_E("dma_auto", "UART DMA initialization failed");
            uart.init_polling();
            uart.put_string("@RESULT mode=dma status=FAIL error=DMA_INIT\n");
            return;
        }

        HAL::UART::Uart0Dma uart_dma{uart};
        if (!uart_dma.init())
        {
            RTT_LOG_E("dma_auto", "EDMA channel initialization failed");
            uart.init_polling();
            uart.put_string("@RESULT mode=dma status=FAIL error=EDMA_INIT\n");
            return;
        }

        std::snprintf(message, sizeof(message),
                      "@READY mode=dma sequence=%u frame=%u payload=%u\n",
                      static_cast<unsigned>(sequence),
                      static_cast<unsigned>(frame_size),
                      static_cast<unsigned>(payload_size));
        uart.put_string(message);
        uart.wait_tx_complete();

        RTT_LOG_I("dma_auto", "READY sent; entering blocking RX");
        const uint32_t rx_started = HAL::PERF::get_cycle_count();

        if (!uart_dma.receive(g_dma_frame, frame_size, TEST_TIMEOUT_LOOPS))
        {
            const uint32_t rx_cycles = HAL::PERF::get_cycle_count() - rx_started;
            RTT_LOG_E("dma_auto", "RX failed after %u PMU cycles",
                      static_cast<unsigned>(rx_cycles));
            log_dma_hw_state("RX_FAIL", REGS::EDMA::CH_UART0_RX);
            uart_dma.stop();
            uart.init_polling();
            std::snprintf(message, sizeof(message),
                          "@RESULT mode=dma sequence=%u status=FAIL error=RX_TIMEOUT\n",
                          static_cast<unsigned>(sequence));
            uart.put_string(message);
            return;
        }

        const uint32_t rx_cycles = HAL::PERF::get_cycle_count() - rx_started;
        RTT_LOG_I("dma_auto", "RX completed after %u PMU cycles",
                  static_cast<unsigned>(rx_cycles));
        log_dma_hw_state("RX_DONE", REGS::EDMA::CH_UART0_RX);

        const auto* header = reinterpret_cast<const DmaFrameHeader*>(g_dma_frame);
        const uint16_t received_crc =
            static_cast<uint16_t>(g_dma_frame[frame_size - 2u]) |
            static_cast<uint16_t>(static_cast<uint16_t>(g_dma_frame[frame_size - 1u]) << 8u);
        const uint16_t calculated_crc =
            crc16_ccitt_false(g_dma_frame, frame_size - DMA_FRAME_CRC_SIZE);

        const bool header_ok =
            header->magic == DMA_FRAME_MAGIC &&
            header->version == DMA_FRAME_VERSION &&
            header->sequence == sequence &&
            header->payload_size == payload_size &&
            header->seed == seed;
        const bool crc_ok = received_crc == calculated_crc;
        const bool data_ok = header_ok && validate_dma_payload(*header);

        RTT_LOG_I("dma_auto",
                  "VALIDATE header=%s crc=%s data=%s received_crc=%04x calculated_crc=%04x",
                  header_ok ? "PASS" : "FAIL",
                  crc_ok ? "PASS" : "FAIL",
                  data_ok ? "PASS" : "FAIL",
                  static_cast<unsigned>(received_crc),
                  static_cast<unsigned>(calculated_crc));

        // Echo the exact bytes received even on a validation failure.  This lets
        // the host locate and report the first corrupted byte.
        const uint32_t tx_started = HAL::PERF::get_cycle_count();
        const bool tx_ok = uart_dma.transmit(g_dma_frame, frame_size, TEST_TIMEOUT_LOOPS);
        const uint32_t tx_cycles = HAL::PERF::get_cycle_count() - tx_started;
        RTT_LOG_I("dma_auto", "TX %s after %u PMU cycles",
                  tx_ok ? "completed" : "failed",
                  static_cast<unsigned>(tx_cycles));
        log_dma_hw_state(tx_ok ? "TX_DONE" : "TX_FAIL", REGS::EDMA::CH_UART0_TX);
        uart_dma.stop();
        uart.init_polling();

        std::snprintf(message, sizeof(message),
                      "@RESULT mode=dma sequence=%u rx=%u tx=%u crc=%s data=%s status=%s\n",
                      static_cast<unsigned>(sequence),
                      static_cast<unsigned>(frame_size),
                      tx_ok ? static_cast<unsigned>(frame_size) : 0u,
                      crc_ok ? "PASS" : "FAIL",
                      data_ok ? "PASS" : "FAIL",
                      (tx_ok && crc_ok && data_ok) ? "PASS" : "FAIL");
        uart.put_string(message);
        RTT_LOG_I("dma_auto", "END seq=%u status=%s",
                  static_cast<unsigned>(sequence),
                  (tx_ok && crc_ok && data_ok) ? "PASS" : "FAIL");
    }

    [[nodiscard]] bool buffer_equals(const char* buffer,
                                     const char* expected,
                                     const std::size_t size) noexcept
    {
        for (std::size_t i = 0u; i < size; ++i)
        {
            if (buffer[i] != expected[i])
                return false;
        }
        return expected[size] == '\0';
    }

    void interrupt_rx_callback(const char c)
    {
        if (!g_interrupt_active || c == '\r' || c == '\n')
            return;

        const std::size_t index = g_interrupt_count;
        if (index < TEST_DATA_SIZE)
        {
            g_interrupt_buffer[index] = c;
            g_interrupt_count = index + 1u;
        }

        if (g_interrupt_count == TEST_DATA_SIZE)
        {
            g_interrupt_active = false;
            g_interrupt_complete = true;
        }
    }

    void reset_interrupt_receiver() noexcept
    {
        g_interrupt_active = false;
        g_interrupt_complete = false;
        g_interrupt_count = 0u;
        for (std::size_t i = 0u; i < TEST_DATA_SIZE; ++i)
            g_interrupt_buffer[i] = '\0';
    }

    void print_test_result(const Uart& uart, const char* name, const bool passed) noexcept
    {
        uart.put_string("[RESULT] ");
        uart.put_string(name);
        uart.put_string(passed ? ": PASS\n" : ": FAIL\n");

        if (passed)
            RTT_LOG_I(TAG, "%s test passed", name);
        else
            RTT_LOG_E(TAG, "%s test failed", name);
    }

    [[nodiscard]] bool test_polling(Uart& uart) noexcept
    {
        PollingRestore restore{uart};
        uart.init_polling();
        char buffer[TEST_DATA_SIZE]{};

        uart.put_string("\n[POLLING] TX uses polling; RX uses polling.\n");
        uart.put_string("Send POLL1234: ");

        std::size_t count = 0u;
        while (count < TEST_DATA_SIZE)
        {
            const char c = uart.get_char();
            if (c == '\r' || c == '\n')
                continue;
            buffer[count++] = c;
            uart.put_char(c);
        }
        uart.put_string("\n");
        return buffer_equals(buffer, POLLING_PATTERN, TEST_DATA_SIZE);
    }

    [[nodiscard]] bool test_interrupt(Uart& uart) noexcept
    {
        PollingRestore restore{uart};
        reset_interrupt_receiver();

        if (!uart.init_interrupt(interrupt_rx_callback))
            return false;

        g_interrupt_active = true;
        uart.put_string("\n[INTERRUPT] TX uses polling; RX uses the UART ISR.\n");
        uart.put_string("Send IRQ12345: ");

        std::size_t echoed = 0u;
        uint32_t timeout = TEST_TIMEOUT_LOOPS;
        while (!g_interrupt_complete && timeout != 0u)
        {
            const std::size_t available = g_interrupt_count;
            while (echoed < available)
                uart.put_char(g_interrupt_buffer[echoed++]);
            --timeout;
            __asm volatile("nop");
        }
        g_interrupt_active = false;

        while (echoed < g_interrupt_count)
            uart.put_char(g_interrupt_buffer[echoed++]);
        uart.put_string("\n");

        if (!g_interrupt_complete || g_interrupt_count != TEST_DATA_SIZE)
            return false;

        char received[TEST_DATA_SIZE]{};
        for (std::size_t i = 0u; i < TEST_DATA_SIZE; ++i)
            received[i] = g_interrupt_buffer[i];
        return buffer_equals(received, INTERRUPT_PATTERN, TEST_DATA_SIZE);
    }

    [[nodiscard]] bool test_dma(Uart& uart) noexcept
    {
        PollingRestore restore{uart};
        if (!uart.init_dma())
            return false;

        alignas(64) uint8_t rx_buffer[64]{};
        bool passed = false;

        {
            HAL::UART::Uart0Dma uart_dma{uart};
            if (!uart_dma.init())
                return false;

            constexpr char prompt[] =
                "\r\n[DMA] TX and RX use UART0 EDMA channels.\r\n"
                "Send DMA12345: ";
            constexpr char newline[] = "\r\n";

            if (!uart_dma.transmit(prompt, sizeof(prompt) - 1u, TEST_TIMEOUT_LOOPS) ||
                !uart_dma.receive(rx_buffer, TEST_DATA_SIZE, TEST_TIMEOUT_LOOPS) ||
                !uart_dma.transmit(rx_buffer, TEST_DATA_SIZE, TEST_TIMEOUT_LOOPS) ||
                !uart_dma.transmit(newline, sizeof(newline) - 1u, TEST_TIMEOUT_LOOPS))
            {
                return false;
            }

            passed = buffer_equals(reinterpret_cast<const char*>(rx_buffer),
                                   DMA_PATTERN,
                                   TEST_DATA_SIZE);
        }
        return passed;
    }

    void print_help(const Uart& uart) noexcept
    {
        uart.put_string(
            "\nCommands:\n"
            "  help             Show this help\n"
            "  test polling     Polling TX + polling RX\n"
            "  test interrupt   Polling TX + ISR RX\n"
            "  test dma         EDMA TX + EDMA RX\n"
            "  test all         Run all three tests\n"
            "  auto dma N S K   Binary DMA loopback: payload size, sequence, seed\n"
            "\nEditing:\n"
            "  Up/Down          Command history\n"
            "  Left/Right       Move cursor\n"
            "  Home/End         Start/end of line\n"
            "  Backspace/Delete Edit line\n"
            "  Ctrl+C           Cancel line\n"
            "  Ctrl+L           Clear screen\n"
            "  Tab              Complete command\n");
    }

    void run_all_tests(Uart& uart) noexcept
    {
        const bool polling_passed = test_polling(uart);
        print_test_result(uart, "polling", polling_passed);
        const bool interrupt_passed = test_interrupt(uart);
        print_test_result(uart, "interrupt (ISR RX + polling TX)", interrupt_passed);
        const bool dma_passed = test_dma(uart);
        print_test_result(uart, "dma", dma_passed);

        uart.put_string("\n[SUMMARY]\n");
        uart.put_string(polling_passed ? "  polling:   PASS\n" : "  polling:   FAIL\n");
        uart.put_string(interrupt_passed ? "  interrupt: PASS\n" : "  interrupt: FAIL\n");
        uart.put_string(dma_passed ? "  dma:       PASS\n" : "  dma:       FAIL\n");
        uart.put_string(polling_passed && interrupt_passed && dma_passed
                            ? "  overall:   PASS\n"
                            : "  overall:   FAIL\n");
    }

    void execute_command(Uart& uart, const char* command) noexcept
    {
        uint32_t payload_size = 0u;
        uint32_t sequence = 0u;
        uint32_t seed = 0u;

        if (strings_equal(command, COMMAND_NAMES[0]))
            print_help(uart);
        else if (strings_equal(command, COMMAND_NAMES[1]))
            print_test_result(uart, "polling", test_polling(uart));
        else if (strings_equal(command, COMMAND_NAMES[2]))
            print_test_result(uart, "interrupt (ISR RX + polling TX)", test_interrupt(uart));
        else if (strings_equal(command, COMMAND_NAMES[3]))
            print_test_result(uart, "dma", test_dma(uart));
        else if (strings_equal(command, COMMAND_NAMES[4]))
            run_all_tests(uart);
        else if (parse_auto_dma(command, payload_size, sequence, seed))
        {
            if (payload_size == 0u || payload_size > DMA_MAX_PAYLOAD_SIZE ||
                sequence > 0xFFFFu)
            {
                uart.put_string("@RESULT mode=dma status=FAIL error=BAD_ARGUMENT\n");
            }
            else
            {
                run_dma_auto_test(uart, payload_size,
                                  static_cast<uint16_t>(sequence), seed);
            }
        }
        else if (command[0] == 'a' && command[1] == 'u' &&
                 command[2] == 't' && command[3] == 'o')
            uart.put_string("@RESULT mode=dma status=FAIL error=BAD_COMMAND\n");
        else if (*command != '\0')
            uart.put_string("Unknown command. Type 'help'.\n");
    }

    [[noreturn]] void run_cli(Uart& uart) noexcept
    {
        char command[LineEditor::COMMAND_BUFFER_SIZE]{};
        LineEditor editor{uart, COMMAND_NAMES, COMMAND_COUNT};

        uart.put_string("\nAM335x UART CLI\n");
        print_help(uart);
        uart.put_string("\n");

        while (true)
        {
            if (editor.read_line(command) == LineEditor::Result::SUBMITTED)
                execute_command(uart, command);
        }
    }
}

int main()
{
    if (!init_board())
    {
        RTT_LOG_E(TAG, "Board initialization failed");
        while (true)
            __asm volatile("wfi");
    }

    RTT_LOG_I(TAG, "UART CLI initialized");
    ::run_cli(Board::get_uart0());

    while (true)
        __asm volatile("wfi");
}
