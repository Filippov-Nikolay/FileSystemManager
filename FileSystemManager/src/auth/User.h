#pragma once

#include <string>

struct User
{
    std::string firstName;
    std::string lastName;
    std::string username;
    std::string passwordHash; // stored as "salt:sha256(salt+password)"
};