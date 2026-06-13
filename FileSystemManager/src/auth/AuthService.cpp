#include "AuthService.h"
#include "PasswordHasher.h"

#include <iostream>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace {

std::string ReadPassword()
{
    std::string password;

#ifdef _WIN32
    int ch;
    while (true)
    {
        ch = _getch();

        if (ch == '\r' || ch == '\n')
            break;

        if (ch == '\b')
        {
            if (!password.empty())
            {
                password.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }

        // Skip extended/special key sequences (arrows, F-keys, etc.)
        if (ch == 0 || ch == 0xE0)
        {
            _getch();
            continue;
        }

        if (ch >= 32 && ch < 127)
        {
            password += static_cast<char>(ch);
            std::cout << '*' << std::flush;
        }
    }
#else
    termios oldt{};
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~static_cast<tcflag_t>(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::getline(std::cin, password);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    std::cout << '\n';
    return password;
}

// Rejects characters that would corrupt the pipe-delimited users.txt format
bool ContainsForbiddenChar(const std::string& s)
{
    return s.find('|') != std::string::npos || s.find('\r') != std::string::npos;
}

} // namespace

AuthService::AuthService(UserRepository& userRepository)
    : userRepository(userRepository)
{
}

void AuthService::Register()
{
    User user;

    std::cout << "First name: ";
    std::cin >> user.firstName;
    if (ContainsForbiddenChar(user.firstName))
    {
        std::cout << "Invalid input: '|' character is not allowed.\n";
        return;
    }

    std::cout << "Last name: ";
    std::cin >> user.lastName;
    if (ContainsForbiddenChar(user.lastName))
    {
        std::cout << "Invalid input: '|' character is not allowed.\n";
        return;
    }

    std::cout << "Username: ";
    std::cin >> user.username;
    if (ContainsForbiddenChar(user.username))
    {
        std::cout << "Invalid input: '|' character is not allowed.\n";
        return;
    }

    // Check for duplicate username before prompting for password
    for (const auto& existing : userRepository.GetAll())
    {
        if (existing.username == user.username)
        {
            std::cout << "Username '" << user.username << "' is already taken.\n";
            return;
        }
    }

    std::cin.ignore();
    std::cout << "Password: ";
    user.passwordHash = ReadPassword();

    user.passwordHash = PasswordHasher::Hash(user.passwordHash);
    userRepository.TrySave(user);

    std::cout << "User registered successfully.\n";
}

std::optional<User> AuthService::Login()
{
    std::string username;
    std::string password;

    std::cout << "Username: ";
    std::cin >> username;

    std::cin.ignore();
    std::cout << "Password: ";
    password = ReadPassword();

    const auto users = userRepository.GetAll();

    for (const auto& user : users)
    {
        if (user.username == username && PasswordHasher::Verify(password, user.passwordHash))
        {
            std::cout << "Login successful. Welcome, " << user.firstName << "!\n";
            return user;
        }
    }

    std::cout << "Invalid username or password.\n";
    return std::nullopt;
}
