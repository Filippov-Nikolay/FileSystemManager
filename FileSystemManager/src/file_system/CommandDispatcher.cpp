#include "CommandDispatcher.h"
#include "../utils/Color.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

CommandDispatcher::CommandDispatcher(FileManager& fileManager, Logger& logger)
    : fileManager(fileManager), logger(logger)
{
    RegisterCommands();
}

void CommandDispatcher::RegisterCommands()
{
    // ── Directories ───────────────────────────────────────────────────────────

    handlers["crdr"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 1)) return DispatchResult::Continue;
            if (fileManager.CreateDirectory(cmd.arguments[0]))
                logger.Info("Directory created: " + cmd.arguments[0]);
            return DispatchResult::Continue;
        },
        "crdr <path>", "Create directory"
    };

    handlers["rndr"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 2)) return DispatchResult::Continue;
            if (fileManager.RenameDirectory(cmd.arguments[0], cmd.arguments[1]))
                logger.Info("Directory renamed: " + cmd.arguments[0] + " -> " + cmd.arguments[1]);
            return DispatchResult::Continue;
        },
        "rndr <from> <to>", "Rename directory"
    };

    handlers["cpdr"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 2)) return DispatchResult::Continue;
            if (fileManager.CopyDirectory(cmd.arguments[0], cmd.arguments[1]))
                logger.Info("Directory copied: " + cmd.arguments[0] + " -> " + cmd.arguments[1]);
            return DispatchResult::Continue;
        },
        "cpdr <from> <to>", "Copy directory"
    };

    handlers["mvdr"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 2)) return DispatchResult::Continue;
            if (fileManager.MoveDirectory(cmd.arguments[0], cmd.arguments[1]))
                logger.Info("Directory moved: " + cmd.arguments[0] + " -> " + cmd.arguments[1]);
            return DispatchResult::Continue;
        },
        "mvdr <from> <to>", "Move directory"
    };

    handlers["rmdr"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 1)) return DispatchResult::Continue;
            std::cout << Color::Yellow << "Delete directory '" << cmd.arguments[0] << "'? [y/N] " << Color::Reset << std::flush;
            char confirm = 'n';
            std::cin.get(confirm);
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (confirm != 'y' && confirm != 'Y') { std::cout << Color::Yellow << "Cancelled." << Color::Reset << '\n'; return DispatchResult::Continue; }
            if (fileManager.DeleteDirectory(cmd.arguments[0]))
                logger.Info("Directory deleted: " + cmd.arguments[0]);
            return DispatchResult::Continue;
        },
        "rmdr <path>", "Delete directory (prompts for confirmation)"
    };

    handlers["lsdr"] = {
        [this](const Command& cmd) -> DispatchResult {
            const auto& path = cmd.arguments.empty() ? "." : cmd.arguments[0];
            if (fileManager.ListDirectory(path))
                logger.Info("Directory listed: " + path);
            return DispatchResult::Continue;
        },
        "lsdr [path]", "List directory (default: current)"
    };

    handlers["szdr"] = {
        [this](const Command& cmd) -> DispatchResult {
            const auto& path = cmd.arguments.empty() ? "." : cmd.arguments[0];
            if (fileManager.GetDirectorySize(path))
                logger.Info("Directory size checked: " + path);
            return DispatchResult::Continue;
        },
        "szdr [path]", "Show directory size (default: current)"
    };

    // ── Files ─────────────────────────────────────────────────────────────────

    handlers["crf"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 1)) return DispatchResult::Continue;
            if (fileManager.CreateFile(cmd.arguments[0]))
                logger.Info("File created: " + cmd.arguments[0]);
            return DispatchResult::Continue;
        },
        "crf <path>", "Create empty file"
    };

    handlers["rnf"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 2)) return DispatchResult::Continue;
            if (fileManager.RenameFile(cmd.arguments[0], cmd.arguments[1]))
                logger.Info("File renamed: " + cmd.arguments[0] + " -> " + cmd.arguments[1]);
            return DispatchResult::Continue;
        },
        "rnf <from> <to>", "Rename file"
    };

    handlers["cpf"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 2)) return DispatchResult::Continue;
            if (fileManager.CopyFile(cmd.arguments[0], cmd.arguments[1]))
                logger.Info("File copied: " + cmd.arguments[0] + " -> " + cmd.arguments[1]);
            return DispatchResult::Continue;
        },
        "cpf <from> <to>", "Copy file"
    };

    handlers["mvf"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 2)) return DispatchResult::Continue;
            if (fileManager.MoveFile(cmd.arguments[0], cmd.arguments[1]))
                logger.Info("File moved: " + cmd.arguments[0] + " -> " + cmd.arguments[1]);
            return DispatchResult::Continue;
        },
        "mvf <from> <to>", "Move file"
    };

    handlers["rmf"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 1)) return DispatchResult::Continue;
            std::cout << Color::Yellow << "Delete file '" << cmd.arguments[0] << "'? [y/N] " << Color::Reset << std::flush;
            char confirm = 'n';
            std::cin.get(confirm);
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (confirm != 'y' && confirm != 'Y') { std::cout << Color::Yellow << "Cancelled." << Color::Reset << '\n'; return DispatchResult::Continue; }
            if (fileManager.DeleteFile(cmd.arguments[0]))
                logger.Info("File deleted: " + cmd.arguments[0]);
            return DispatchResult::Continue;
        },
        "rmf <path>", "Delete file (prompts for confirmation)"
    };

    handlers["szf"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 1)) return DispatchResult::Continue;
            if (fileManager.GetFileSize(cmd.arguments[0]))
                logger.Info("File size checked: " + cmd.arguments[0]);
            return DispatchResult::Continue;
        },
        "szf <path>", "Show file size"
    };

    handlers["cat"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 1)) return DispatchResult::Continue;
            if (fileManager.ReadFile(cmd.arguments[0]))
                logger.Info("File read: " + cmd.arguments[0]);
            return DispatchResult::Continue;
        },
        "cat <path>", "Display file contents"
    };

    handlers["write"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 2)) return DispatchResult::Continue;
            std::string content;
            for (size_t i = 1; i < cmd.arguments.size(); ++i)
            {
                if (i > 1) content += ' ';
                content += cmd.arguments[i];
            }
            if (fileManager.WriteFile(cmd.arguments[0], content))
                logger.Info("File written: " + cmd.arguments[0]);
            return DispatchResult::Continue;
        },
        "write <path> <text>", "Write single line to file (overwrites)"
    };

    handlers["append"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 2)) return DispatchResult::Continue;
            std::string content;
            for (size_t i = 1; i < cmd.arguments.size(); ++i)
            {
                if (i > 1) content += ' ';
                content += cmd.arguments[i];
            }
            if (fileManager.AppendFile(cmd.arguments[0], content))
                logger.Info("File appended: " + cmd.arguments[0]);
            return DispatchResult::Continue;
        },
        "append <path> <text>", "Append single line to file"
    };

    // ── Navigation ────────────────────────────────────────────────────────────

    handlers["pwd"] = {
        [this](const Command&) -> DispatchResult {
            fileManager.PrintCurrentDirectory();
            return DispatchResult::Continue;
        },
        "pwd", "Show current directory"
    };

    handlers["cd"] = {
        [this](const Command& cmd) -> DispatchResult {
            const auto path = cmd.arguments.empty() ? "" : cmd.arguments[0];
            if (fileManager.ChangeDirectory(path))
                logger.Info("Directory changed: " + path);
            return DispatchResult::Continue;
        },
        "cd [path]", "Change directory (no arg = workspace root)"
    };

    handlers["shmsk"] = {
        [this](const Command& cmd) -> DispatchResult {
            if (!HasArguments(cmd, 1)) return DispatchResult::Continue;
            const auto path = cmd.arguments.size() >= 2 ? cmd.arguments[0] : ".";
            const auto& mask = cmd.arguments.size() >= 2 ? cmd.arguments[1] : cmd.arguments[0];
            if (fileManager.SearchByMask(path, mask))
                logger.Info("Search by mask: " + path + " " + mask);
            return DispatchResult::Continue;
        },
        "shmsk [path] <mask>", "Search files by mask (* and ? wildcards, e.g. shmsk *.txt)"
    };

    // ── System ────────────────────────────────────────────────────────────────

    handlers["help"] = {
        [this](const Command&) -> DispatchResult {
            ShowHelp();
            return DispatchResult::Continue;
        },
        "help", "Show this help"
    };

    handlers["clear"] = {
        [](const Command&) -> DispatchResult {
            // ANSI VT100: clear screen + move cursor to top-left
            std::cout << "\033[2J\033[H" << std::flush;
            return DispatchResult::Continue;
        },
        "clear", "Clear the screen"
    };

    handlers["clfm"] = {
        [this](const Command&) -> DispatchResult {
            logger.Info("File manager closed");
            std::cout << "File manager closed.\n";
            return DispatchResult::Quit;
        },
        "clfm", "Close file manager"
    };

    // Build sorted command name list (single source of truth for tab completion)
    commandNames.reserve(handlers.size());
    for (const auto& [name, _] : handlers)
        commandNames.push_back(name);
    std::sort(commandNames.begin(), commandNames.end());
}

// ─────────────────────────────────────────────────────────────────────────────

DispatchResult CommandDispatcher::Dispatch(const Command& command)
{
    if (command.name.empty()) return DispatchResult::Continue;

    const auto it = handlers.find(command.name);
    if (it == handlers.end())
    {
        std::cout << Color::Red << "Unknown command '" << command.name << "'." << Color::Reset << " Type 'help' for a list.\n";
        logger.Error("Unknown command: " + command.name);
        return DispatchResult::Continue;
    }

    return it->second.handler(command);
}

const std::vector<std::string>& CommandDispatcher::GetCommandNames() const
{
    return commandNames;
}

// ─────────────────────────────────────────────────────────────────────────────

void CommandDispatcher::ShowHelp() const
{
    const std::vector<std::pair<std::string, std::vector<std::string>>> categories = {
        {"Directories", {"crdr", "rndr", "cpdr", "mvdr", "rmdr", "lsdr", "szdr"}},
        {"Files",       {"crf",  "rnf",  "cpf",  "mvf",  "rmf",  "szf", "cat", "write", "append"}},
        {"Navigation",  {"pwd",  "cd",   "shmsk"}},
        {"System",      {"help", "clear", "clfm"}},
    };

    std::cout << '\n' << Color::Bold << "Available commands:" << Color::Reset << '\n';

    for (const auto& [category, names] : categories)
    {
        std::cout << '\n' << Color::Cyan << Color::Bold << "  " << category << ':' << Color::Reset << '\n';
        for (const auto& name : names)
        {
            const auto it = handlers.find(name);
            if (it == handlers.end()) continue;
            std::cout << "    " << Color::White << std::left << std::setw(30)
                      << it->second.usage << Color::Reset << it->second.description << '\n';
        }
    }

    std::cout << '\n' << Color::Cyan << Color::Bold << "  Tips:" << Color::Reset << '\n';
    std::cout << "    " << Color::Yellow << "Tab" << Color::Reset << "          Autocomplete commands and paths\n";
    std::cout << "    " << Color::Yellow << "Up / Down" << Color::Reset << "    Navigate command history (persisted across sessions)\n";
    std::cout << "    " << Color::Yellow << "Quotes" << Color::Reset << "       Use \"my folder\" for paths with spaces\n";
}

bool CommandDispatcher::HasArguments(const Command& command, int count)
{
    if (static_cast<int>(command.arguments.size()) < count)
    {
        std::cout << Color::Red << "Usage error: '" << command.name << "' requires " << count << " argument(s)." << Color::Reset << '\n';
        logger.Error("Invalid argument count for command: " + command.name);
        return false;
    }
    return true;
}
