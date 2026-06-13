#include "Application.h"

#include "../auth/AuthService.h"
#include "../auth/UserRepository.h"
#include "../file_system/CommandParser.h"
#include "../file_system/FileManager.h"
#include "../file_system/CommandDispatcher.h"
#include "../utils/ReadLine.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

static constexpr size_t kMaxHistory = 500;

Application::Application()
    : logger("logs.txt")
{
}

// ─────────────────────────────────────────────────────────────────────────────

void Application::Run()
{
    logger.Info("Application started");

    const auto user = RunAuthLoop();

    if (!user)
    {
        logger.Info("Application stopped");
        return;
    }

    RunFileManagerLoop(*user);

    logger.Info("Application stopped");
}

// ─────────────────────────────────────────────────────────────────────────────

std::optional<User> Application::RunAuthLoop()
{
    UserRepository userRepository("users.txt");
    AuthService authService(userRepository);

    while (true)
    {
        std::cout << "1. Login\n";
        std::cout << "2. Register\n";
        std::cout << "3. Exit\n";
        std::cout << "Choose option: ";

        int option = 0;

        if (!(std::cin >> option))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Please enter a number.\n";
            continue;
        }

        if (option == 1)
        {
            auto user = authService.Login();
            if (user)
            {
                logger.Info("User logged in: " + user->username);
                return user;
            }
            logger.Error("Failed login attempt");
        }
        else if (option == 2)
        {
            authService.Register();
            logger.Info("User registered");
        }
        else if (option == 3)
        {
            std::cout << "Application closed.\n";
            return std::nullopt;
        }
        else
        {
            std::cout << "Invalid option.\n";
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void Application::RunFileManagerLoop(const User& user)
{
    FileManager fileManager("workspace");
    CommandParser parser;
    CommandDispatcher dispatcher(fileManager, logger);

    // Load persistent history; keep only the most recent entries
    std::vector<std::string> history;
    {
        std::ifstream histFile("history.txt");
        std::string line;
        while (std::getline(histFile, line))
            if (!line.empty()) history.push_back(line);

        if (history.size() > kMaxHistory)
            history.erase(history.begin(), history.end() - kMaxHistory);
    }

    // Completer: command names for first token, filesystem paths for arguments
    Completer completer = [&](const std::string& prefix, const std::string& linePrefix) -> std::vector<std::string>
    {
        const bool isFirstToken = linePrefix.find_first_not_of(' ') == std::string::npos;

        if (isFirstToken)
        {
            std::vector<std::string> results;
            for (const auto& cmd : dispatcher.GetCommandNames())
                if (cmd.substr(0, prefix.size()) == prefix)
                    results.push_back(cmd);
            return results;
        }

        return fileManager.GetCompletions(prefix);
    };

    std::cout << "\nFile manager started.\n";
    std::cout << "Type 'help' to show available commands.\n";
    std::cout << "Type 'clfm' to close file manager.\n";

    while (true)
    {
        const std::string prompt =
            "\n" + user.username + "@" + fileManager.GetDisplayPath() + "> ";

        const std::string input = ReadLine(prompt, completer, &history);

        const Command command = parser.Parse(input);

        if (dispatcher.Dispatch(command) == DispatchResult::Quit)
            break;
    }

    // Persist history (cap to kMaxHistory before writing)
    {
        const size_t start = history.size() > kMaxHistory ? history.size() - kMaxHistory : 0;
        std::ofstream histFile("history.txt", std::ios::trunc);
        for (size_t i = start; i < history.size(); ++i)
            histFile << history[i] << '\n';
    }
}
