#ifndef UART_CLI_LINE_EDITOR_HPP
#define UART_CLI_LINE_EDITOR_HPP

#include <cstddef>
#include <cstdint>
#include "console.hpp"

class LineEditor final
{
public:
    static constexpr std::size_t HISTORY_DEPTH = 8u;
    static constexpr  std::size_t COMMAND_BUFFER_SIZE = 48u;

    enum class Result : uint8_t
    {
        SUBMITTED,
        CANCELLED
    };

    LineEditor(SHELL::Console& uart,
               const char* const* completions,
               std::size_t completion_count,
               const char* prompt) noexcept
        : m_uart(uart),
          m_completions(completions),
          m_completion_count(completion_count),
          m_prompt(prompt) {}

    [[nodiscard]] Result read_line(char* command) noexcept;

private:
    SHELL::Console& m_uart;
    const char* const* m_completions;
    std::size_t m_completion_count;
    const char* m_prompt;
    char m_history[HISTORY_DEPTH][COMMAND_BUFFER_SIZE]{};
    std::size_t m_history_count{0u};
    bool m_ignore_lf{false};

    void redraw(const char* command, std::size_t length, std::size_t cursor) noexcept;
    void load_line(char* command, std::size_t& length, std::size_t& cursor, const char* source) noexcept;
    void add_to_history(const char* command) noexcept;
    void history_up(char* command, std::size_t& length, std::size_t& cursor, std::size_t& history_view, char* draft) noexcept;
    void history_down(char* command, std::size_t& length, std::size_t& cursor, std::size_t& history_view, const char* draft) noexcept;
    void erase_at_cursor(char* command, std::size_t& length, std::size_t cursor) noexcept;
    void handle_escape(char* command, std::size_t& length, std::size_t& cursor, std::size_t& history_view, char* draft) noexcept;
    void complete(char* command, std::size_t& length, std::size_t& cursor) noexcept;
};

[[nodiscard]] bool strings_equal(const char* lhs, const char* rhs) noexcept;

#endif //UART_CLI_LINE_EDITOR_HPP
