#pragma once

#include <span>
#include <string>
#include <vector>
#include <cstddef>
#include "gitx/types.hpp"

namespace gitx {

class Hash {
public:
    static std::string sha256(std::span<const std::byte> data);
    static std::string compute_id(ObjectType type, std::span<const std::byte> content);
};

} // namespace gitx
