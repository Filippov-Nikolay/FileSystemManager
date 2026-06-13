#include "UserRepository.h"

#include <fstream>
#include <sstream>

UserRepository::UserRepository(const std::string& filePath)
    : filePath(filePath)
{
}

bool UserRepository::TrySave(const User& user)
{
    // Check for existing username while reading
    {
        std::ifstream readFile(filePath);
        std::string line;

        while (std::getline(readFile, line))
        {
            std::stringstream ss(line);
            std::string fn, ln, uname;
            std::getline(ss, fn, '|');
            std::getline(ss, ln, '|');
            std::getline(ss, uname, '|');

            if (uname == user.username)
                return false;
        }
    }

    std::ofstream outFile(filePath, std::ios::app);
    if (!outFile.is_open())
        return false;

    outFile << user.firstName << '|'
            << user.lastName  << '|'
            << user.username  << '|'
            << user.passwordHash << '\n';

    return true;
}

std::vector<User> UserRepository::GetAll()
{
    std::vector<User> users;

    std::ifstream file(filePath);
    if (!file.is_open())
        return users;

    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream stream(line);

        User user;

        std::getline(stream, user.firstName,    '|');
        std::getline(stream, user.lastName,     '|');
        std::getline(stream, user.username,     '|');
        std::getline(stream, user.passwordHash, '|');

        if (!user.username.empty())
            users.push_back(user);
    }

    return users;
}
