#pragma once

#include "Command.h"

#include <string>

class CommandParser
{
public:
    Command Parse(const std::string& input);
};