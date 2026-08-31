#ifndef UART_SHELL_SHELL_HPP
#define UART_SHELL_SHELL_HPP

#include <cstddef>
#include <cstdint>
#include "console.hpp"
#include "line_editor.hpp"

namespace SHELL
{
    class Shell final
    {
    public:
        using Handler = void (*)(void* context, const char* command) noexcept;
        enum class Visibility : uint8_t { VISIBLE, HIDDEN };

        template<class Uart>
        explicit Shell(Uart& uart, const char* prompt = "uart> ") noexcept
            : m_console(uart),
              m_editor(m_console, m_completions, MAX_COMMANDS, prompt) {}

        [[nodiscard]] bool add_command(const char* name, Handler handler,
                                       void* context,
                                       Visibility visibility = Visibility::VISIBLE) noexcept;
        [[nodiscard]] Console& console() noexcept { return m_console; }
        [[noreturn]] void run() noexcept;

    private:
        struct Command
        {
            const char* name;
            Handler handler;
            void* context;
            Visibility visibility;
        };

        static constexpr std::size_t MAX_COMMANDS = 16u;
        Console m_console;
        const char* m_completions[MAX_COMMANDS]{};
        LineEditor m_editor;
        Command m_commands[MAX_COMMANDS]{};
        std::size_t m_command_count{0u};
        std::size_t m_visible_count{0u};

        [[nodiscard]] const Command* find_command(const char* line) const noexcept;
    };
}

#endif
