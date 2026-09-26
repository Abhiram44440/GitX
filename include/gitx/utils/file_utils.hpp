#pragma once

#include <filesystem>
#include <vector>
#include <cstddef>
#include <string>

namespace gitx {

std::vector<std::byte> read_file_bytes(const std::filesystem::path& path);
std::string read_file_text(const std::filesystem::path& path);
void write_file_bytes(const std::filesystem::path& path, const std::vector<std::byte>& data);
void write_file_text(const std::filesystem::path& path, const std::string& text);

// List files recursively, excluding .gitx/
std::vector<std::filesystem::path> list_files(const std::filesystem::path& dir);

} // namespace gitx
