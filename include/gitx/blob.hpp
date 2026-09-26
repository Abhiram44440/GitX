#pragma once

#include "gitx/object.hpp"
#include <vector>
#include <cstddef>

namespace gitx {

class Blob : public Object {
public:
    explicit Blob(std::vector<std::byte> content);

    ObjectType type() const override;
    std::vector<std::byte> serialize() const override;
    const std::vector<std::byte>& content() const;

private:
    std::vector<std::byte> content_;
};

} // namespace gitx
