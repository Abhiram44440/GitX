#include "gitx/utils/serialization.hpp"
#include <stdexcept>

namespace gitx {

void write_string(std::vector<std::byte>& buf, const std::string& s) {
    write_u32(buf, static_cast<uint32_t>(s.size()));
    for (char c : s) {
        buf.push_back(static_cast<std::byte>(c));
    }
}

std::string read_string(const std::vector<std::byte>& buf, size_t& offset) {
    uint32_t len = read_u32(buf, offset);
    if (offset + len > buf.size()) {
        throw std::runtime_error("Buffer underflow reading string");
    }
    std::string s(reinterpret_cast<const char*>(buf.data() + offset), len);
    offset += len;
    return s;
}

void write_u32(std::vector<std::byte>& buf, uint32_t value) {
    buf.push_back(static_cast<std::byte>((value >> 24) & 0xFF));
    buf.push_back(static_cast<std::byte>((value >> 16) & 0xFF));
    buf.push_back(static_cast<std::byte>((value >> 8) & 0xFF));
    buf.push_back(static_cast<std::byte>(value & 0xFF));
}

uint32_t read_u32(const std::vector<std::byte>& buf, size_t& offset) {
    if (offset + 4 > buf.size()) {
        throw std::runtime_error("Buffer underflow reading u32");
    }
    uint32_t value = 0;
    value |= static_cast<uint32_t>(buf[offset]) << 24;
    value |= static_cast<uint32_t>(buf[offset + 1]) << 16;
    value |= static_cast<uint32_t>(buf[offset + 2]) << 8;
    value |= static_cast<uint32_t>(buf[offset + 3]);
    offset += 4;
    return value;
}

void write_u64(std::vector<std::byte>& buf, uint64_t value) {
    buf.push_back(static_cast<std::byte>((value >> 56) & 0xFF));
    buf.push_back(static_cast<std::byte>((value >> 48) & 0xFF));
    buf.push_back(static_cast<std::byte>((value >> 40) & 0xFF));
    buf.push_back(static_cast<std::byte>((value >> 32) & 0xFF));
    buf.push_back(static_cast<std::byte>((value >> 24) & 0xFF));
    buf.push_back(static_cast<std::byte>((value >> 16) & 0xFF));
    buf.push_back(static_cast<std::byte>((value >> 8) & 0xFF));
    buf.push_back(static_cast<std::byte>(value & 0xFF));
}

uint64_t read_u64(const std::vector<std::byte>& buf, size_t& offset) {
    if (offset + 8 > buf.size()) {
        throw std::runtime_error("Buffer underflow reading u64");
    }
    uint64_t value = 0;
    value |= static_cast<uint64_t>(buf[offset]) << 56;
    value |= static_cast<uint64_t>(buf[offset + 1]) << 48;
    value |= static_cast<uint64_t>(buf[offset + 2]) << 40;
    value |= static_cast<uint64_t>(buf[offset + 3]) << 32;
    value |= static_cast<uint64_t>(buf[offset + 4]) << 24;
    value |= static_cast<uint64_t>(buf[offset + 5]) << 16;
    value |= static_cast<uint64_t>(buf[offset + 6]) << 8;
    value |= static_cast<uint64_t>(buf[offset + 7]);
    offset += 8;
    return value;
}

} // namespace gitx
