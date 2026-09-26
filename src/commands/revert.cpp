#include "gitx/commands/commands.hpp"
#include "gitx/repository.hpp"
#include "gitx/utils/file_utils.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace gitx {
namespace {

bool is_object_id(const std::string& id) {
    return id.size() == 64 && std::all_of(id.begin(), id.end(), [](unsigned char c) {
        return std::isxdigit(c) != 0;
    });
}

std::pair<ObjectId, std::string> parse_commit(const std::vector<std::byte>& data) {
    const std::string text = from_bytes(data);
    const auto first_newline = text.find('\n');
    const auto separator = text.find("\n\n");
    if (first_newline == std::string::npos || separator == std::string::npos ||
        text.rfind("tree ", 0) != 0) {
        throw std::runtime_error("Corrupt commit object");
    }

    const auto tree_id = text.substr(5, first_newline - 5);
    if (!is_object_id(tree_id)) throw std::runtime_error("Corrupt commit object");

    const auto message_start = separator + 2;
    const auto message_end = text.find('\n', message_start);
    return {tree_id, text.substr(message_start, message_end - message_start)};
}

void restore_tree(Repository& repo, const ObjectId& tree_id,
                  const std::filesystem::path& directory, const std::string& prefix,
                  Index& target_index) {
    auto [type, data] = repo.object_store().read(tree_id);
    if (type != ObjectType::Tree) throw std::runtime_error("Expected tree object: " + tree_id);

    const std::string text = from_bytes(data);
    size_t offset = 0;
    while (offset < text.size()) {
        const auto nul = text.find('\0', offset);
        if (nul == std::string::npos) throw std::runtime_error("Corrupt tree object: " + tree_id);
        const auto newline = text.find('\n', nul + 1);
        if (newline == std::string::npos) throw std::runtime_error("Corrupt tree object: " + tree_id);

        const auto first_space = text.find(' ', offset);
        const auto second_space = first_space == std::string::npos ? std::string::npos
                                                                    : text.find(' ', first_space + 1);
        if (first_space == std::string::npos || second_space == std::string::npos ||
            second_space >= nul) {
            throw std::runtime_error("Corrupt tree object: " + tree_id);
        }

        const auto mode = text.substr(offset, first_space - offset);
        const auto type_name = text.substr(first_space + 1, second_space - first_space - 1);
        const auto name = text.substr(second_space + 1, nul - second_space - 1);
        const auto id = text.substr(nul + 1, newline - nul - 1);
        if (name.empty() || name == "." || name == ".." || name.find_first_of("/\\") != std::string::npos ||
            !is_object_id(id) ||
            (type_name != "blob" && type_name != "tree") ||
            (type_name == "blob" && mode != "100644") ||
            (type_name == "tree" && mode != "040000")) {
            throw std::runtime_error("Corrupt tree object: " + tree_id);
        }

        const auto path = prefix.empty() ? name : prefix + "/" + name;
        const auto full_path = directory / name;
        if (type_name == "tree") {
            std::filesystem::create_directories(full_path);
            restore_tree(repo, id, full_path, path, target_index);
        } else {
            auto [blob_type, blob_data] = repo.object_store().read(id);
            if (blob_type != ObjectType::Blob) throw std::runtime_error("Expected blob object: " + id);
            if (target_index.has_entry(path)) throw std::runtime_error("Corrupt tree object: " + tree_id);
            write_file_bytes(full_path, blob_data);
            const auto mtime = static_cast<uint64_t>(
                std::filesystem::last_write_time(full_path).time_since_epoch().count());
            target_index.add_entry(path, IndexEntry{
                id,
                static_cast<uint32_t>(std::filesystem::file_size(full_path)),
                mtime,
            });
        }

        offset = newline + 1;
    }
}

} // namespace

int cmd_revert(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        std::cerr << "Usage: gitx revert <commit_hash>\n";
        return 1;
    }

    try {
        auto repo = Repository::find();
        if (!repo) {
            std::cerr << "fatal: not a gitx repository\n";
            return 1;
        }
        if (!is_object_id(args[2])) throw std::runtime_error("Invalid commit hash: " + args[2]);

        auto [commit_type, commit_data] = repo->object_store().read(args[2]);
        if (commit_type != ObjectType::Commit) throw std::runtime_error("Expected commit object: " + args[2]);
        auto [tree_id, message] = parse_commit(commit_data);

        Index target_index;
        restore_tree(*repo, tree_id, repo->root(), "", target_index);

        repo->load_index();
        for (const auto& [path, _] : repo->index().entries()) {
            if (!target_index.has_entry(path)) {
                std::filesystem::remove(repo->root() / path);
            }
        }

        repo->index() = std::move(target_index);
        repo->save_index();
        repo->write_head(args[2]);
        std::cout << "HEAD is now at " << args[2].substr(0, 7) << " " << message << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace gitx
