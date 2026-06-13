#pragma once

#include "../auth/User.h"
#include "../logging/Logger.h"

#include <optional>

class Application
{
public:
    Application();
    void Run();

private:
    Logger logger;

    std::optional<User> RunAuthLoop();
    void RunFileManagerLoop(const User& user);
};
