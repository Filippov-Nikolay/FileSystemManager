#include "tests/TestFramework.h"

// Each header registers its TEST() cases via static initialisation.
// Order here is the order tests run.
#include "tests/CommandParserTests.h"
#include "tests/FileManagerTests.h"
#include "tests/PasswordHasherTests.h"
#include "tests/UserRepositoryTests.h"

int main()
{
    return RunAllTests();
}
