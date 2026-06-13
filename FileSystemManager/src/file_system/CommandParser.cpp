#include "CommandParser.h"

Command CommandParser::Parse(const std::string& input)
{
    Command command;
    std::string token;
    bool inQuotes = false;

    auto flush = [&]()
    {
        if (!token.empty())
        {
            if (command.name.empty())
                command.name = token;
            else
                command.arguments.push_back(token);

            token.clear();
        }
    };

    for (const char c : input)
    {
        if (c == '"')
        {
            inQuotes = !inQuotes;
        }
        else if (c == ' ' && !inQuotes)
        {
            flush();
        }
        else
        {
            token += c;
        }
    }

    flush();

    return command;
}
