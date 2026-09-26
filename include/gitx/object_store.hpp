#pragma once

#include "gitx/types.hpp"
#include <filesystem>
#include <vector>
#include <cstddef>

namespace gitx {

class ObjectStore {
public:
    explicit ObjectStore(const std::filesystem::path& gitx_dir);

    ObjectId write(ObjectType type, const std::vector<std::byte>& content);
    bool exists(const ObjectId& id) const;
    std::pair<ObjectType, std::vector<std::byte>> read(const ObjectId& id) const;

private:
    std::filesystem::path objects_dir_;
};

} // namespace gitx
