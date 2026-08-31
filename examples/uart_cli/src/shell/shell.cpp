#include "shell.hpp"

namespace SHELL
{
    namespace
    {
        [[nodiscard]] bool command_matches(const char* line,
                                           const char* name) noexcept
        {
            while (*name != '\0' && *line == *name)
            {
                ++line;
                ++name;
            }
            return *name == '\0' && (*line == '\0' || *line == ' ');
        }
    }

    bool Shell::add_command(const char* name, const Handler handler,
                            void* context, const Visibility visibility) noexcept
    {
        if (name == nullptr || *name == '\0' || handler == nullptr ||
            m_command_count == MAX_COMMANDS)
        {
            return false;
        }

        m_commands[m_command_count++] = {name, handler, context, visibility};
        if (visibility == Visibility::VISIBLE)
            m_completions[m_visible_count++] = name;
        return true;
    }

    const Shell::Command* Shell::find_command(const char* line) const noexcept
    {
        const Command* result = nullptr;
        std::size_t longest = 0u;
        for (std::size_t i = 0u; i < m_command_count; ++i)
        {
            const char* name = m_commands[i].name;
            if (!command_matches(line, name))
                continue;
            std::size_t length = 0u;
            while (name[length] != '\0')
                ++length;
            if (length > longest)
            {
                longest = length;
                result = &m_commands[i];
            }
        }
        return result;
    }

    [[noreturn]] void Shell::run() noexcept
    {
        char line[LineEditor::COMMAND_BUFFER_SIZE]{};
        while (true)
        {
            if (m_editor.read_line(line) != LineEditor::Result::SUBMITTED)
                continue;
            if (*line == '\0')
                continue;
            const Command* command = find_command(line);
            if (command != nullptr)
                command->handler(command->context, line);
            else
                m_console.put_string("Unknown command. Type 'help'.\n");
        }
    }
}
