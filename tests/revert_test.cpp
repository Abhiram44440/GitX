#include "gitx/blob.hpp"
#include "gitx/commit.hpp"
#include "gitx/commands/commands.hpp"
#include "gitx/repository.hpp"
#include "gitx/utils/file_utils.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace gitx {
int cmd_revert(const std::vector<std::string>& args);
}

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

class CurrentDirectory {
public:
    explicit CurrentDirectory(const std::filesystem::path& path)
        : old_(std::filesystem::current_path()) {
        std::filesystem::current_path(path);
    }

    ~CurrentDirectory() { std::filesystem::current_path(old_); }

private:
    std::filesystem::path old_;
};

int run_status(std::string& output) {
    std::ostringstream captured;
    auto* old = std::cout.rdbuf(captured.rdbuf());
    const int result = gitx::cmd_status({"gitx", "status"});
    std::cout.rdbuf(old);
    output = captured.str();
    return result;
}

std::unordered_map<std::string, gitx::IndexEntry> snapshot_index(gitx::Repository& repo) {
    repo.load_index();
    return repo.index().entries();
}

bool same_index(const std::unordered_map<std::string, gitx::IndexEntry>& left,
                const std::unordered_map<std::string, gitx::IndexEntry>& right) {
    if (left.size() != right.size()) return false;
    for (const auto& [path, entry] : left) {
        auto it = right.find(path);
        if (it == right.end() || it->second.blob_id != entry.blob_id) return false;
    }
    return true;
}

} // namespace

int main() {
    const auto root = std::filesystem::temp_directory_path() /
        ("gitx-revert-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));

    try {
        std::filesystem::create_directories(root);
        CurrentDirectory cwd(root);
        require(gitx::cmd_init({"gitx", "init"}) == 0, "init failed");

        gitx::write_file_text("original.txt", "original\n");
        gitx::write_file_text("deleted.txt", "deleted\n");
        require(gitx::cmd_add({"gitx", "add", "original.txt"}) == 0, "add original failed");
        require(gitx::cmd_add({"gitx", "add", "deleted.txt"}) == 0, "add deleted failed");
        require(gitx::cmd_commit({"gitx", "commit", "-m", "commit A"}) == 0, "commit A failed");

        auto repo = gitx::Repository::find();
        require(repo.has_value(), "repository not found");
        const auto commit_a = *repo->read_head();
        const auto index_a = snapshot_index(*repo);
        const auto blob_a = repo->index().get_entry("original.txt").blob_id;

        gitx::write_file_text("original.txt", "changed\n");
        gitx::write_file_text("after.txt", "after\n");
        std::filesystem::remove("deleted.txt");
        require(gitx::cmd_add({"gitx", "add", "original.txt"}) == 0, "update original failed");
        require(gitx::cmd_add({"gitx", "add", "after.txt"}) == 0, "add after failed");
        repo->load_index();
        repo->index().remove_entry("deleted.txt");
        repo->save_index();
        require(gitx::cmd_commit({"gitx", "commit", "-m", "commit B"}) == 0, "commit B failed");

        require(gitx::cmd_revert({"gitx", "revert", commit_a}) == 0, "revert to A failed");
        require(gitx::read_file_text("original.txt") == "original\n", "original file was not restored");
        require(std::filesystem::exists("deleted.txt"), "deleted file was not restored");
        require(!std::filesystem::exists("after.txt"), "post-target file was not removed");
        require(*repo->read_head() == commit_a, "HEAD was not moved to A");
        require(same_index(snapshot_index(*repo), index_a), "index does not match target tree");

        std::string status;
        require(run_status(status) == 0, "status failed after revert");
        require(status == "nothing to commit, working tree clean\n", "reverted tree is not clean");

        const auto files_before_noop = gitx::read_file_bytes("original.txt");
        const auto index_before_noop = snapshot_index(*repo);
        require(gitx::cmd_revert({"gitx", "revert", commit_a}) == 0, "revert to HEAD failed");
        require(gitx::read_file_bytes("original.txt") == files_before_noop, "HEAD no-op changed file");
        require(same_index(snapshot_index(*repo), index_before_noop), "HEAD no-op changed index");

        require(gitx::cmd_revert({"gitx", "revert", std::string(64, '0')}) != 0,
                "unknown commit was accepted");
        require(gitx::cmd_revert({"gitx", "revert", blob_a}) != 0,
                "blob was accepted as commit");

        const auto corrupt_tree = repo->object_store().write(gitx::ObjectType::Tree, gitx::to_bytes("bad tree\n"));
        gitx::Commit corrupt_commit(corrupt_tree, std::nullopt, "test", "now", "corrupt");
        const auto corrupt_commit_id = repo->object_store().write(gitx::ObjectType::Commit, corrupt_commit.serialize());
        require(gitx::cmd_revert({"gitx", "revert", corrupt_commit_id}) != 0,
                "corrupt tree was accepted");
    } catch (const std::exception& e) {
        std::cerr << "revert test failed: " << e.what() << "\n";
        std::filesystem::remove_all(root);
        return 1;
    }

    std::filesystem::remove_all(root);
    return 0;
}
