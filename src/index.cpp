#include "gitx/index.hpp"
#include "gitx/utils/serialization.hpp"
#include "gitx/utils/file_utils.hpp"
#include <fstream>
#include <algorithm>
#include <stdexcept>

namespace gitx {

Index Index::load(const std::filesystem::path& index_path) {
    Index idx;
    if (!std::filesystem::exists(index_path)) return idx;

    auto data = read_file_bytes(index_path);
    size_t offset = 0;

    uint32_t version = read_u32(data, offset);
    if (version != INDEX_VERSION) {
        throw std::runtime_error("Invalid index version");
    }

    uint32_t count = read_u32(data, offset);

    for (uint32_t i = 0; i < count; ++i) {
        std::string blob_id;
        for (int j = 0; j < 64; ++j) {
            char c = static_cast<char>(data[offset++]);
            if (c != '\0') blob_id += c;
        }
        uint32_t file_size = read_u32(data, offset);
        uint64_t mtime = read_u64(data, offset);
        std::string path = read_string(data, offset);

        idx.entries_[path] = IndexEntry{blob_id, file_size, mtime};
    }

    return idx;
}

void Index::save(const std::filesystem::path& index_path) const {
    std::vector<std::byte> buf;

    write_u32(buf, INDEX_VERSION);
    write_u32(buf, static_cast<uint32_t>(entries_.size()));

    auto sorted = sorted_paths();
    for (const auto& path : sorted) {
        const auto& entry = entries_.at(path);
        // Pad blob_id to exactly 64 bytes
        for (size_t i = 0; i < 64; ++i) {
            if (i < entry.blob_id.size()) {
                buf.push_back(static_cast<std::byte>(entry.blob_id[i]));
            } else {
                buf.push_back(std::byte{0});
            }
        }
        write_u32(buf, entry.file_size);
        write_u64(buf, entry.mtime);
        write_string(buf, path);
    }

    auto parent = index_path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    write_file_bytes(index_path, buf);
}

void Index::add_entry(const std::string& path, const IndexEntry& entry) {
    entries_[path] = entry;
}

void Index::remove_entry(const std::string& path) {
    entries_.erase(path);
}

bool Index::has_entry(const std::string& path) const {
    return entries_.count(path) > 0;
}

const IndexEntry& Index::get_entry(const std::string& path) const {
    return entries_.at(path);
}

const std::unordered_map<std::string, IndexEntry>& Index::entries() const {
    return entries_;
}

std::vector<std::string> Index::sorted_paths() const {
    std::vector<std::string> paths;
    paths.reserve(entries_.size());
    for (const auto& [path, _] : entries_) {
        paths.push_back(path);
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

bool Index::empty() const { return entries_.empty(); }

} // namespace gitx
