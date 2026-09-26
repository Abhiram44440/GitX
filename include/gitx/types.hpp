#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace gitx {

using ObjectId = std::string; // 64-char hex SHA-256

enum class ObjectType : uint8_t {
    Blob,
    Tree,
    Commit,
};

inline const char* to_string(ObjectType t) {
    switch (t) {
        case ObjectType::Blob: return "blob";
        case ObjectType::Tree: return "tree";
        case ObjectType::Commit: return "commit";
    }
    return "unknown";
}

inline ObjectType object_type_from_string(const std::string& s) {
    if (s == "blob") return ObjectType::Blob;
    if (s == "tree") return ObjectType::Tree;
    if (s == "commit") return ObjectType::Commit;
    throw std::runtime_error("Unknown object type: " + s);
}

// MSVC-safe helpers: char <-> std::byte conversions
inline std::vector<std::byte> to_bytes(const std::string& s) {
    std::vector<std::byte> result;
    result.reserve(s.size());
    for (char c : s) result.push_back(static_cast<std::byte>(c));
    return result;
}

inline std::string from_bytes(const std::vector<std::byte>& bytes) {
    std::string result;
    result.reserve(bytes.size());
    for (auto b : bytes) result += static_cast<char>(b);
    return result;
}

} // namespace gitx
