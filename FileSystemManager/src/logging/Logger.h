#pragma once

#include <fstream>
#include <string>

class Logger
{
public:
    explicit Logger(const std::string& filePath);

    void Info(const std::string& message);
    void Error(const std::string& message);

private:
    std::ofstream file;

    void Write(const std::string& level, const std::string& message);
};