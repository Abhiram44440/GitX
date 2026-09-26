#include "gitx/object_store.hpp"
#include "gitx/hash.hpp"
#include "gitx/utils/file_utils.hpp"
#include <stdexcept>
#include <fstream>

namespace gitx {

ObjectStore::ObjectStore(const std::filesystem::path& gitx_dir)
    : objects_dir_(gitx_dir / "objects") {
    std::filesystem::create_directories(objects_dir_);
}

ObjectId ObjectStore::write(ObjectType type, const std::vector<std::byte>& content) {
    ObjectId id = Hash::compute_id(type, content);
    if (exists(id)) return id;

    auto dir = objects_dir_ / id.substr(0, 2);
    std::filesystem::create_directories(dir);

    auto file_path = dir / id.substr(2);
    std::ofstream ofs(file_path, std::ios::binary);
    if (!ofs) throw std::runtime_error("Failed to write object: " + id);

    // Write header: "type\0" then content
    std::string header = to_string(type);
    header += '\0';
    ofs.write(header.data(), static_cast<std::streamsize>(header.size()));
    ofs.write(reinterpret_cast<const char*>(content.data()),
              static_cast<std::streamsize>(content.size()));
    return id;
}

bool ObjectStore::exists(const ObjectId& id) const {
    if (id.size() < 3) return false;
    auto file_path = objects_dir_ / id.substr(0, 2) / id.substr(2);
    return std::filesystem::exists(file_path);
}

std::pair<ObjectType, std::vector<std::byte>> ObjectStore::read(const ObjectId& id) const {
    if (!exists(id)) {
        throw std::runtime_error("Object not found: " + id);
    }

    auto file_path = objects_dir_ / id.substr(0, 2) / id.substr(2);
    auto data = read_file_bytes(file_path);

    // Parse header: "type\0"
    size_t null_pos = 0;
    while (null_pos < data.size() && data[null_pos] != std::byte('\0')) {
        ++null_pos;
    }
    if (null_pos >= data.size()) {
        throw std::runtime_error("Corrupt object: " + id);
    }

    std::string type_str(reinterpret_cast<const char*>(data.data()), null_pos);
    ObjectType type = object_type_from_string(type_str);

    std::vector<std::byte> content(data.begin() + null_pos + 1, data.end());
    return {type, std::move(content)};
}

} // namespace gitx
