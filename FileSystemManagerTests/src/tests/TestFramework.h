#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ─── Registration ─────────────────────────────────────────────────────────────

struct TestCase
{
    const char* name;
    void (*fn)();
};

inline std::vector<TestCase>& Registry()
{
    static std::vector<TestCase> cases;
    return cases;
}

struct RegisterTest
{
    RegisterTest(const char* name, void (*fn)())
    {
        Registry().push_back({name, fn});
    }
};

// ─── Macros ───────────────────────────────────────────────────────────────────

#define TEST(name)                                                    \
    static void _test_##name();                                       \
    static RegisterTest _reg_##name(#name, _test_##name);             \
    static void _test_##name()

#define ASSERT_TRUE(expr)                                             \
    do {                                                              \
        if (!(expr))                                                  \
            throw std::runtime_error("ASSERT_TRUE failed: " #expr);  \
    } while (0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_EQ(a, b)                                               \
    do {                                                              \
        auto _a = (a); auto _b = (b);                                 \
        if (!(_a == _b)) {                                            \
            std::ostringstream _oss;                                  \
            _oss << "ASSERT_EQ failed: " #a " == " #b                \
                 << "\n    lhs = " << _a                              \
                 << "\n    rhs = " << _b;                             \
            throw std::runtime_error(_oss.str());                     \
        }                                                             \
    } while (0)

// ─── Output helpers ───────────────────────────────────────────────────────────

// Redirects cout to a buffer; str() retrieves captured output
struct CaptureOutput
{
    std::ostringstream buffer;
    std::streambuf*    saved;

    CaptureOutput()  : saved(std::cout.rdbuf(buffer.rdbuf())) {}
    ~CaptureOutput() { std::cout.rdbuf(saved); }

    std::string str() const { return buffer.str(); }
};

// Discards all cout output within the scope
struct SuppressOutput
{
    std::ostringstream sink;
    std::streambuf*    saved;

    SuppressOutput()  : saved(std::cout.rdbuf(sink.rdbuf())) {}
    ~SuppressOutput() { std::cout.rdbuf(saved); }
};

// ─── Runner ───────────────────────────────────────────────────────────────────

inline int RunAllTests()
{
    int passed = 0, failed = 0;

    for (const auto& tc : Registry())
    {
        try
        {
            tc.fn();
            std::cout << "  \033[32m[PASS]\033[0m " << tc.name << '\n';
            ++passed;
        }
        catch (const std::exception& e)
        {
            std::cout << "  \033[31m[FAIL]\033[0m " << tc.name
                      << "\n         " << e.what() << '\n';
            ++failed;
        }
    }

    std::cout << '\n';

    if (failed == 0)
        std::cout << "\033[32mall " << passed << " tests passed\033[0m\n";
    else
        std::cout << "\033[31m" << passed << " passed, " << failed << " FAILED\033[0m\n";

    return failed > 0 ? 1 : 0;
}
