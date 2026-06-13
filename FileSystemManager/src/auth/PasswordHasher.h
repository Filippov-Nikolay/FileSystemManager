#pragma once

#include <string>

class PasswordHasher
{
public:
    // Returns "salt:sha256(salt+password)"
    static std::string Hash(const std::string& password);

    // Verifies against a stored "salt:hash" entry; falls back to plain SHA-256 for legacy entries
    static bool Verify(const std::string& password, const std::string& stored);

private:
    static std::string GenerateSalt();
    static std::string ComputeSHA256(const std::string& input);
};
