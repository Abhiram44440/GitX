#pragma once

#include <vector>
#include <cstddef>
#include <string>
#include <cstdint>

namespace gitx {

// Write a length-prefixed string to a byte buffer
void write_string(std::vector<std::byte>& buf, const std::string& s);
std::string read_string(const std::vector<std::byte>& buf, size_t& offset);

// Write a 32-bit big-endian integer
void write_u32(std::vector<std::byte>& buf, uint32_t value);
uint32_t read_u32(const std::vector<std::byte>& buf, size_t& offset);

// Write a 64-bit big-endian integer
void write_u64(std::vector<std::byte>& buf, uint64_t value);
uint64_t read_u64(const std::vector<std::byte>& buf, size_t& offset);

} // namespace gitx
