#include "gitx/repository.hpp"
#include "gitx/utils/file_utils.hpp"
#include <stdexcept>
#include <fstream>

namespace gitx {

Repository::Repository(std::filesystem::path root, std::filesystem::path gitx_dir)
    : root_(std::move(root))
    , gitx_dir_(std::move(gitx_dir))
    , object_store_(gitx_dir_)
    , index_(Index{}) {}

std::optional<Repository> Repository::find() {
    auto cwd = std::filesystem::current_path();
    auto dir = cwd;

    while (true) {
        auto gitx_dir = dir / ".gitx";
        if (std::filesystem::exists(gitx_dir) && std::filesystem::is_directory(gitx_dir)) {
            return Repository(dir, gitx_dir);
        }
        if (dir == dir.parent_path()) break;
        dir = dir.parent_path();
    }
    return std::nullopt;
}

Repository Repository::init(const std::filesystem::path& path) {
    auto gitx_dir = path / ".gitx";
    if (std::filesystem::exists(gitx_dir)) {
        throw std::runtime_error("fatal: .gitx/ already exists");
    }

    std::filesystem::create_directories(gitx_dir / "objects");
    std::filesystem::create_directories(gitx_dir / "refs" / "heads");

    // Create empty HEAD
    write_file_text(gitx_dir / "HEAD", "");

    // Create empty config
    write_file_text(gitx_dir / "config", "");

    return Repository(path, gitx_dir);
}

const std::filesystem::path& Repository::root() const { return root_; }
const std::filesystem::path& Repository::gitx_dir() const { return gitx_dir_; }
ObjectStore& Repository::object_store() { return object_store_; }
const ObjectStore& Repository::object_store() const { return object_store_; }

std::optional<ObjectId> Repository::read_head() const {
    auto head_path = gitx_dir_ / "HEAD";
    if (!std::filesystem::exists(head_path)) return std::nullopt;

    auto content = read_file_text(head_path);
    // Trim whitespace
    while (!content.empty() && (content.back() == '\n' || content.back() == '\r')) {
        content.pop_back();
    }
    if (content.empty()) return std::nullopt;
    return content;
}

void Repository::write_head(const ObjectId& commit_id) {
    write_file_text(gitx_dir_ / "HEAD", commit_id);
}

Index& Repository::index() { return index_; }
const Index& Repository::index() const { return index_; }

void Repository::load_index() {
    index_ = Index::load(gitx_dir_ / "index");
}

void Repository::save_index() const {
    index_.save(gitx_dir_ / "index");
}

} // namespace gitx
