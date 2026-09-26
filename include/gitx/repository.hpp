#pragma once

#include "gitx/object_store.hpp"
#include "gitx/index.hpp"
#include <filesystem>
#include <optional>

namespace gitx {

class Repository {
public:
    // Find .gitx/ by walking up from cwd
    static std::optional<Repository> find();

    // Create new repo
    static Repository init(const std::filesystem::path& path);

    // Accessors
    const std::filesystem::path& root() const;
    const std::filesystem::path& gitx_dir() const;
    ObjectStore& object_store();
    const ObjectStore& object_store() const;

    // HEAD management
    std::optional<ObjectId> read_head() const;
    void write_head(const ObjectId& commit_id);

    // Index
    Index& index();
    const Index& index() const;
    void load_index();
    void save_index() const;

private:
    Repository(std::filesystem::path root, std::filesystem::path gitx_dir);

    std::filesystem::path root_;
    std::filesystem::path gitx_dir_;
    ObjectStore object_store_;
    Index index_;
};

} // namespace gitx
