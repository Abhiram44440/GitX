#pragma once

#include "gitx/types.hpp"
#include "gitx/object.hpp"
#include <string>
#include <optional>
#include <chrono>

namespace gitx {

class Commit : public Object {
public:
    Commit(ObjectId tree_id,
           std::optional<ObjectId> parent_id,
           std::string author,
           std::string timestamp,
           std::string message);

    ObjectType type() const override;
    std::vector<std::byte> serialize() const override;

    const ObjectId& tree_id() const;
    const std::optional<ObjectId>& parent_id() const;
    const std::string& author() const;
    const std::string& timestamp() const;
    const std::string& message() const;

private:
    ObjectId tree_id_;
    std::optional<ObjectId> parent_id_;
    std::string author_;
    std::string timestamp_;
    std::string message_;
};

} // namespace gitx
