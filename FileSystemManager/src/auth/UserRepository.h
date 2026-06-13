#pragma once

#include "User.h"

#include <string>
#include <vector>

class UserRepository
{
public:
    explicit UserRepository(const std::string& filePath);

    // Returns false if username already exists, true on success
    bool TrySave(const User& user);
    std::vector<User> GetAll();

private:
    std::string filePath;
};
