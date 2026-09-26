#include "gitx/blob.hpp"

namespace gitx {

Blob::Blob(std::vector<std::byte> content)
    : content_(std::move(content)) {}

ObjectType Blob::type() const { return ObjectType::Blob; }

std::vector<std::byte> Blob::serialize() const { return content_; }

const std::vector<std::byte>& Blob::content() const { return content_; }

} // namespace gitx
