#pragma once

#include "User.h"
#include "UserRepository.h"

#include <optional>

class AuthService
{
public:
    explicit AuthService(UserRepository& userRepository);

    void Register();
    std::optional<User> Login();

private:
    UserRepository& userRepository;
};