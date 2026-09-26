#include "gitx/tree.hpp"
#include <algorithm>
#include <cstring>

namespace gitx {

Tree::Tree(std::vector<TreeEntry> entries)
    : entries_(std::move(entries)) {
    std::sort(entries_.begin(), entries_.end());
}

ObjectType Tree::type() const { return ObjectType::Tree; }

std::vector<std::byte> Tree::serialize() const {
    std::vector<std::byte> buf;
    for (const auto& entry : entries_) {
        std::string mode_str;
        switch (entry.type) {
            case ObjectType::Blob:   mode_str = "100644"; break;
            case ObjectType::Tree:   mode_str = "040000"; break;
            case ObjectType::Commit: mode_str = "160000"; break;
        }
        // Format: "mode type name\0id_binary"
        // But for simplicity, we use text format: "mode type name\tid\n"
        // Actually let's use a simple binary format for robustness.
        // Format: mode(6 bytes) + space + type + space + name + \0 + id_bytes(32)
        // We'll use text for readability and determinism.
        std::string line = mode_str + " " + to_string(entry.type) + " " + entry.name
                         + std::string(1, '\0') + entry.id + "\n";
        for (char c : line) {
            buf.push_back(static_cast<std::byte>(c));
        }
    }
    return buf;
}

const std::vector<TreeEntry>& Tree::entries() const { return entries_; }

} // namespace gitx
