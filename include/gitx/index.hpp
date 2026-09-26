#pragma once

#include "gitx/types.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace gitx {

struct IndexEntry {
    ObjectId blob_id;
    uint32_t file_size;
    uint64_t mtime;
};

class Index {
public:
    static Index load(const std::filesystem::path& index_path);
    void save(const std::filesystem::path& index_path) const;

    void add_entry(const std::string& path, const IndexEntry& entry);
    void remove_entry(const std::string& path);
    bool has_entry(const std::string& path) const;
    const IndexEntry& get_entry(const std::string& path) const;

    const std::unordered_map<std::string, IndexEntry>& entries() const;
    std::vector<std::string> sorted_paths() const;

    bool empty() const;

private:
    static constexpr uint32_t INDEX_VERSION = 1;
    std::unordered_map<std::string, IndexEntry> entries_;
};

} // namespace gitx
