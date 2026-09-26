#include "gitx/commit.hpp"
#include "gitx/types.hpp"

namespace gitx {

Commit::Commit(ObjectId tree_id,
               std::optional<ObjectId> parent_id,
               std::string author,
               std::string timestamp,
               std::string message)
    : tree_id_(std::move(tree_id))
    , parent_id_(std::move(parent_id))
    , author_(std::move(author))
    , timestamp_(std::move(timestamp))
    , message_(std::move(message)) {}

ObjectType Commit::type() const { return ObjectType::Commit; }

std::vector<std::byte> Commit::serialize() const {
    std::string s;
    s += "tree " + tree_id_ + "\n";
    if (parent_id_) {
        s += "parent " + *parent_id_ + "\n";
    }
    s += "author " + author_ + "\n";
    s += "timestamp " + timestamp_ + "\n";
    s += "\n";
    s += message_;
    if (!message_.empty() && message_.back() != '\n') {
        s += "\n";
    }
    return to_bytes(s);
}

const ObjectId& Commit::tree_id() const { return tree_id_; }
const std::optional<ObjectId>& Commit::parent_id() const { return parent_id_; }
const std::string& Commit::author() const { return author_; }
const std::string& Commit::timestamp() const { return timestamp_; }
const std::string& Commit::message() const { return message_; }

} // namespace gitx
