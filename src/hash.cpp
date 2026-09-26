#include "gitx/hash.hpp"
#include <array>
#include <cstring>
#include <sstream>
#include <iomanip>

namespace gitx {

namespace sha256_detail {

static constexpr std::array<uint32_t, 64> K = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

inline uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
inline uint32_t Ch(uint32_t e, uint32_t f, uint32_t g) { return (e & f) ^ (~e & g); }
inline uint32_t Maj(uint32_t a, uint32_t b, uint32_t c) { return (a & b) ^ (a & c) ^ (b & c); }
inline uint32_t Sigma0(uint32_t a) { return rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22); }
inline uint32_t Sigma1(uint32_t e) { return rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25); }
inline uint32_t sigma0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
inline uint32_t sigma1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

inline uint32_t be32(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

inline void put_be32(uint8_t* p, uint32_t v) {
    p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v;
}

struct State {
    uint32_t h[8];
    uint64_t total_len;
    uint8_t buf[64];
    size_t buf_len;
};

inline void init(State& s) {
    s.h[0] = 0x6a09e667; s.h[1] = 0xbb67ae85;
    s.h[2] = 0x3c6ef372; s.h[3] = 0xa54ff53a;
    s.h[4] = 0x510e527f; s.h[5] = 0x9b05688c;
    s.h[6] = 0x1f83d9ab; s.h[7] = 0x5be0cd19;
    s.total_len = 0;
    s.buf_len = 0;
}

inline void process_block(State& s, const uint8_t block[64]) {
    uint32_t w[64];
    for (int i = 0; i < 16; ++i) w[i] = be32(block + i * 4);
    for (int i = 16; i < 64; ++i)
        w[i] = sigma1(w[i-2]) + w[i-7] + sigma0(w[i-15]) + w[i-16];

    uint32_t a = s.h[0], b = s.h[1], c = s.h[2], d = s.h[3];
    uint32_t e = s.h[4], f = s.h[5], g = s.h[6], h = s.h[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t T1 = h + Sigma1(e) + Ch(e, f, g) + K[i] + w[i];
        uint32_t T2 = Sigma0(a) + Maj(a, b, c);
        h = g; g = f; f = e; e = d + T1;
        d = c; c = b; b = a; a = T1 + T2;
    }
    s.h[0] += a; s.h[1] += b; s.h[2] += c; s.h[3] += d;
    s.h[4] += e; s.h[5] += f; s.h[6] += g; s.h[7] += h;
}

inline void update(State& s, const uint8_t* data, size_t len) {
    s.total_len += len;
    size_t offset = 0;
    if (s.buf_len > 0) {
        size_t need = 64 - s.buf_len;
        size_t take = (len < need) ? len : need;
        std::memcpy(s.buf + s.buf_len, data, take);
        s.buf_len += take;
        offset += take;
        if (s.buf_len == 64) {
            process_block(s, s.buf);
            s.buf_len = 0;
        }
    }
    while (offset + 64 <= len) {
        process_block(s, data + offset);
        offset += 64;
    }
    if (offset < len) {
        std::memcpy(s.buf, data + offset, len - offset);
        s.buf_len = len - offset;
    }
}

inline std::string finalize(State& s) {
    // Pad: append 1-bit, zeros, then 64-bit big-endian bit count
    uint64_t bit_len = s.total_len * 8;

    // Append 0x80
    uint8_t pad_byte = 0x80;
    // Directly pad without going through update (to avoid total_len changes)
    size_t pad_offset = s.buf_len;
    s.buf[pad_offset++] = 0x80;

    // If not enough room for the 8-byte length, fill current block and process
    if (pad_offset > 56) {
        while (pad_offset < 64) s.buf[pad_offset++] = 0;
        process_block(s, s.buf);
        pad_offset = 0;
    }

    // Fill zeros up to 56
    while (pad_offset < 56) s.buf[pad_offset++] = 0;

    // Append 64-bit big-endian bit length
    s.buf[56] = (bit_len >> 56) & 0xff;
    s.buf[57] = (bit_len >> 48) & 0xff;
    s.buf[58] = (bit_len >> 40) & 0xff;
    s.buf[59] = (bit_len >> 32) & 0xff;
    s.buf[60] = (bit_len >> 24) & 0xff;
    s.buf[61] = (bit_len >> 16) & 0xff;
    s.buf[62] = (bit_len >> 8) & 0xff;
    s.buf[63] = bit_len & 0xff;
    process_block(s, s.buf);

    uint8_t hash[32];
    for (int i = 0; i < 8; ++i) put_be32(hash + i * 4, s.h[i]);

    std::ostringstream ss;
    for (int i = 0; i < 32; ++i)
        ss << std::hex << std::setfill('0') << std::setw(2) << int(hash[i]);
    return ss.str();
}

} // namespace sha256_detail

std::string Hash::sha256(std::span<const std::byte> data) {
    sha256_detail::State s;
    sha256_detail::init(s);
    sha256_detail::update(s, reinterpret_cast<const uint8_t*>(data.data()), data.size());
    return sha256_detail::finalize(s);
}

std::string Hash::compute_id(ObjectType type, std::span<const std::byte> content) {
    std::string type_str = to_string(type);
    std::vector<std::byte> header;
    header.reserve(type_str.size() + 1 + content.size());
    for (char c : type_str) header.push_back(static_cast<std::byte>(c));
    header.push_back(static_cast<std::byte>('\0'));
    header.insert(header.end(), content.begin(), content.end());
    return sha256(header);
}

} // namespace gitx
