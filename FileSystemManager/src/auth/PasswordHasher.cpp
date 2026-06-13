#include "PasswordHasher.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>

namespace {

constexpr std::array<uint32_t, 64> K = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

constexpr uint32_t RotR(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

constexpr uint32_t BigSigma0(uint32_t x) { return RotR(x, 2)  ^ RotR(x, 13) ^ RotR(x, 22); }
constexpr uint32_t BigSigma1(uint32_t x) { return RotR(x, 6)  ^ RotR(x, 11) ^ RotR(x, 25); }
constexpr uint32_t SmSigma0(uint32_t x)  { return RotR(x, 7)  ^ RotR(x, 18) ^ (x >> 3);    }
constexpr uint32_t SmSigma1(uint32_t x)  { return RotR(x, 17) ^ RotR(x, 19) ^ (x >> 10);   }
constexpr uint32_t Ch(uint32_t x, uint32_t y, uint32_t z)  { return (x & y) ^ (~x & z);          }
constexpr uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }

void ProcessBlock(const uint8_t* block, std::array<uint32_t, 8>& state)
{
    std::array<uint32_t, 64> W{};

    for (int i = 0; i < 16; ++i)
    {
        W[i] = (static_cast<uint32_t>(block[i * 4])     << 24) |
               (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(block[i * 4 + 2]) <<  8) |
                static_cast<uint32_t>(block[i * 4 + 3]);
    }

    for (int i = 16; i < 64; ++i)
        W[i] = SmSigma1(W[i - 2]) + W[i - 7] + SmSigma0(W[i - 15]) + W[i - 16];

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int i = 0; i < 64; ++i)
    {
        const uint32_t T1 = h + BigSigma1(e) + Ch(e, f, g) + K[i] + W[i];
        const uint32_t T2 = BigSigma0(a) + Maj(a, b, c);
        h = g; g = f; f = e; e = d + T1;
        d = c; c = b; b = a; a = T1 + T2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────

std::string PasswordHasher::ComputeSHA256(const std::string& input)
{
    const auto* data = reinterpret_cast<const uint8_t*>(input.data());
    const uint64_t bitLen = static_cast<uint64_t>(input.size()) * 8;

    std::array<uint32_t, 8> state = {
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
    };

    size_t pos = 0;
    for (; pos + 64 <= input.size(); pos += 64)
        ProcessBlock(data + pos, state);

    const size_t remaining = input.size() - pos;
    const size_t padLen = (remaining < 56) ? 64 : 128;

    std::array<uint8_t, 128> padded{};
    std::memcpy(padded.data(), data + pos, remaining);
    padded[remaining] = 0x80;

    for (int i = 0; i < 8; ++i)
        padded[padLen - 8 + i] = static_cast<uint8_t>(bitLen >> (56 - i * 8));

    ProcessBlock(padded.data(), state);
    if (padLen == 128)
        ProcessBlock(padded.data() + 64, state);

    std::ostringstream oss;
    for (const auto& word : state)
        oss << std::hex << std::setw(8) << std::setfill('0') << word;

    return oss.str();
}

std::string PasswordHasher::GenerateSalt()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned> dist(0, 255);

    std::ostringstream oss;
    for (int i = 0; i < 16; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << dist(gen);
    return oss.str();
}

std::string PasswordHasher::Hash(const std::string& password)
{
    const std::string salt = GenerateSalt();
    return salt + ":" + ComputeSHA256(salt + password);
}

bool PasswordHasher::Verify(const std::string& password, const std::string& stored)
{
    const auto colon = stored.find(':');

    // Legacy entries (no salt) — plain SHA-256 comparison
    if (colon == std::string::npos)
        return ComputeSHA256(password) == stored;

    const std::string salt = stored.substr(0, colon);
    const std::string hash = stored.substr(colon + 1);
    return ComputeSHA256(salt + password) == hash;
}
