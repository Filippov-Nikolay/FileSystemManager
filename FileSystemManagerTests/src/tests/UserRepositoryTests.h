#pragma once

#include "TestFramework.h"
#include "auth/UserRepository.h"
#include "auth/PasswordHasher.h"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// ─── Fixture ──────────────────────────────────────────────────────────────────

struct TempUsersFile
{
    fs::path path;

    TempUsersFile()
    {
        const auto ts = std::chrono::system_clock::now().time_since_epoch().count();
        path = fs::temp_directory_path() / ("fsm_users_" + std::to_string(ts) + ".txt");
    }

    ~TempUsersFile()
    {
        std::error_code ec;
        fs::remove(path, ec);
    }

    UserRepository make() { return UserRepository(path.string()); }
};

// ─── GetAll ───────────────────────────────────────────────────────────────────

TEST(UserRepository_GetAll_EmptyFile_ReturnsEmptyVector)
{
    TempUsersFile f;
    auto repo = f.make();

    const auto users = repo.GetAll();
    ASSERT_EQ(users.size(), 0u);
}

TEST(UserRepository_GetAll_AfterSave_ReturnsSavedUser)
{
    TempUsersFile f;
    auto repo = f.make();

    User u;
    u.firstName   = "Alice";
    u.lastName    = "Smith";
    u.username    = "alice";
    u.passwordHash = PasswordHasher::Hash("secret");

    repo.TrySave(u);

    const auto users = repo.GetAll();
    ASSERT_EQ(users.size(), 1u);
}

// ─── TrySave ─────────────────────────────────────────────────────────────────

TEST(UserRepository_TrySave_NewUser_ReturnsTrue)
{
    TempUsersFile f;
    auto repo = f.make();

    User u;
    u.firstName = u.lastName = "A";
    u.username = "newuser";
    u.passwordHash = PasswordHasher::Hash("pw");

    ASSERT_TRUE(repo.TrySave(u));
}

TEST(UserRepository_TrySave_DuplicateUsername_ReturnsFalse)
{
    TempUsersFile f;
    auto repo = f.make();

    User u;
    u.firstName = u.lastName = "A";
    u.username = "taken";
    u.passwordHash = PasswordHasher::Hash("pw");

    ASSERT_TRUE(repo.TrySave(u));

    User dup;
    dup.firstName = dup.lastName = "B";
    dup.username = "taken";
    dup.passwordHash = PasswordHasher::Hash("other");

    ASSERT_FALSE(repo.TrySave(dup));
}

TEST(UserRepository_TrySave_DifferentUsernames_BothSucceed)
{
    TempUsersFile f;
    auto repo = f.make();

    auto make_user = [](const std::string& name) {
        User u;
        u.firstName = u.lastName = name;
        u.username = name;
        u.passwordHash = PasswordHasher::Hash("pw");
        return u;
    };

    ASSERT_TRUE(repo.TrySave(make_user("alice")));
    ASSERT_TRUE(repo.TrySave(make_user("bob")));

    ASSERT_EQ(repo.GetAll().size(), 2u);
}

// ─── Field preservation ───────────────────────────────────────────────────────

TEST(UserRepository_TrySave_AllFieldsPreservedAfterReload)
{
    TempUsersFile f;
    auto repo = f.make();

    User u;
    u.firstName   = "Ivan";
    u.lastName    = "Petrov";
    u.username    = "ipetrov";
    u.passwordHash = PasswordHasher::Hash("mypass");

    repo.TrySave(u);

    // Reload via fresh repository instance pointing at the same file
    UserRepository repo2(f.path.string());
    const auto loaded = repo2.GetAll();

    ASSERT_EQ(loaded.size(), 1u);
    ASSERT_EQ(loaded[0].firstName,   std::string("Ivan"));
    ASSERT_EQ(loaded[0].lastName,    std::string("Petrov"));
    ASSERT_EQ(loaded[0].username,    std::string("ipetrov"));
    ASSERT_TRUE(PasswordHasher::Verify("mypass", loaded[0].passwordHash));
}

TEST(UserRepository_TrySave_DuplicateDoesNotCorruptFile)
{
    TempUsersFile f;
    auto repo = f.make();

    User u;
    u.firstName = u.lastName = "C";
    u.username = "charlie";
    u.passwordHash = PasswordHasher::Hash("pw");

    repo.TrySave(u);
    repo.TrySave(u); // second attempt must be rejected silently

    ASSERT_EQ(repo.GetAll().size(), 1u);
}
