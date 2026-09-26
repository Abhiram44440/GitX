#pragma once

#include "gitx/types.hpp"
#include "gitx/object.hpp"
#include <string>
#include <vector>

namespace gitx {

struct TreeEntry {
    ObjectType type;
    std::string name;
    ObjectId id;

    bool operator<(const TreeEntry& other) const {
        return name < other.name;
    }
};

class Tree : public Object {
public:
    explicit Tree(std::vector<TreeEntry> entries);

    ObjectType type() const override;
    std::vector<std::byte> serialize() const override;
    const std::vector<TreeEntry>& entries() const;

private:
    std::vector<TreeEntry> entries_;
};

} // namespace gitx
