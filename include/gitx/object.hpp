#pragma once

#include "gitx/types.hpp"
#include <vector>
#include <cstddef>

namespace gitx {

class Object {
public:
    virtual ~Object() = default;
    virtual ObjectType type() const = 0;
    virtual std::vector<std::byte> serialize() const = 0;
};

} // namespace gitx
