#pragma once

#include "TestFramework.h"
#include "auth/PasswordHasher.h"

// ─── Hash format ──────────────────────────────────────────────────────────────

TEST(PasswordHasher_Hash_ContainsSeparator)
{
    const auto stored = PasswordHasher::Hash("secret");
    ASSERT_TRUE(stored.find(':') != std::string::npos);
}

TEST(PasswordHasher_Hash_SaltIs32HexChars)
{
    const auto stored = PasswordHasher::Hash("secret");
    const auto colon  = stored.find(':');
    ASSERT_TRUE(colon != std::string::npos);
    ASSERT_EQ(colon, 32u);
}

TEST(PasswordHasher_Hash_HashPartIs64HexChars)
{
    const auto stored = PasswordHasher::Hash("secret");
    const auto colon  = stored.find(':');
    ASSERT_TRUE(colon != std::string::npos);
    ASSERT_EQ(stored.size() - colon - 1, 64u);
}

// ─── Verify — correct password ───────────────────────────────────────────────

TEST(PasswordHasher_Verify_CorrectPassword_ReturnsTrue)
{
    const auto stored = PasswordHasher::Hash("my-password");
    ASSERT_TRUE(PasswordHasher::Verify("my-password", stored));
}

TEST(PasswordHasher_Verify_WrongPassword_ReturnsFalse)
{
    const auto stored = PasswordHasher::Hash("correct");
    ASSERT_FALSE(PasswordHasher::Verify("wrong", stored));
}

TEST(PasswordHasher_Verify_EmptyPassword_MatchesHashOfEmpty)
{
    const auto stored = PasswordHasher::Hash("");
    ASSERT_TRUE(PasswordHasher::Verify("", stored));
}

TEST(PasswordHasher_Verify_EmptyPassword_DoesNotMatchNonEmpty)
{
    const auto stored = PasswordHasher::Hash("notempty");
    ASSERT_FALSE(PasswordHasher::Verify("", stored));
}

// ─── Salt randomness ─────────────────────────────────────────────────────────

TEST(PasswordHasher_Hash_TwoCalls_ProduceDifferentSalts)
{
    const auto h1 = PasswordHasher::Hash("same");
    const auto h2 = PasswordHasher::Hash("same");
    // Same password → different stored values because salt is random
    ASSERT_FALSE(h1 == h2);
}

TEST(PasswordHasher_Hash_TwoCalls_BothVerifyCorrectly)
{
    const auto h1 = PasswordHasher::Hash("pass");
    const auto h2 = PasswordHasher::Hash("pass");
    ASSERT_TRUE(PasswordHasher::Verify("pass", h1));
    ASSERT_TRUE(PasswordHasher::Verify("pass", h2));
}

// ─── Legacy SHA-256 fallback (no salt prefix) ────────────────────────────────

// SHA-256("hello") per FIPS 180-4
TEST(PasswordHasher_Verify_LegacyPlainHash_hello)
{
    const std::string legacy = "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824";
    ASSERT_TRUE(PasswordHasher::Verify("hello", legacy));
}

TEST(PasswordHasher_Verify_LegacyPlainHash_WrongInput_ReturnsFalse)
{
    const std::string legacy = "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824";
    ASSERT_FALSE(PasswordHasher::Verify("world", legacy));
}
