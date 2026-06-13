#pragma once

#include "Command.h"
#include "FileManager.h"
#include "../logging/Logger.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

enum class DispatchResult { Continue, Quit };

class CommandDispatcher
{
    struct CommandEntry
    {
        std::function<DispatchResult(const Command&)> handler;
        std::string usage;
        std::string description;
    };

public:
    CommandDispatcher(FileManager& fileManager, Logger& logger);

    DispatchResult Dispatch(const Command& command);
    const std::vector<std::string>& GetCommandNames() const;

private:
    FileManager& fileManager;
    Logger& logger;

    std::unordered_map<std::string, CommandEntry> handlers;
    std::vector<std::string> commandNames;

    void RegisterCommands();
    bool HasArguments(const Command& command, int count);
    void ShowHelp() const;
};
