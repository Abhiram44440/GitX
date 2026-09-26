#include "gitx/commands/commands.hpp"
#include "gitx/repository.hpp"
#include "gitx/hash.hpp"
#include "gitx/blob.hpp"
#include "gitx/utils/file_utils.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace gitx {

int cmd_status(const std::vector<std::string>& args) {
    try {
        auto repo = Repository::find();
        if (!repo) {
            std::cerr << "fatal: not a gitx repository\n";
            return 1;
        }

        repo->load_index();
        auto head = repo->read_head();

        // Collect working tree files
        auto wt_files = list_files(repo->root());

        // Get index entries
        auto sorted_paths = repo->index().sorted_paths();

        // Get HEAD tree entries (if HEAD exists)
        std::unordered_map<std::string, ObjectId> head_entries;
        if (head) {
            auto [head_type, head_data] = repo->object_store().read(*head);
            std::string commit_text = from_bytes(head_data);
            size_t pos = commit_text.find("tree ");
            if (pos != std::string::npos) {
                size_t end = commit_text.find('\n', pos);
                std::string tree_id = commit_text.substr(pos + 5, end - pos - 5);
                auto [tree_type, tree_data] = repo->object_store().read(tree_id);
                std::string tree_text = from_bytes(tree_data);
                std::istringstream iss(tree_text);
                std::string line;
                while (std::getline(iss, line)) {
                    if (line.empty()) continue;
                    auto null_pos = line.find('\0');
                    if (null_pos == std::string::npos) continue;
                    std::string header = line.substr(0, null_pos);
                    std::string id = line.substr(null_pos + 1);
                    auto sp1 = header.find(' ');
                    auto sp2 = header.find(' ', sp1 + 1);
                    std::string name = header.substr(sp2 + 1);
                    head_entries[name] = id;
                }
            }
        }

        // Collect statuses
        std::vector<std::string> to_commit;
        std::vector<std::string> not_staged;
        std::vector<std::string> untracked;

        for (const auto& path : sorted_paths) {
            const auto& entry = repo->index().get_entry(path);
            auto full_path = repo->root() / path;

            if (!std::filesystem::exists(full_path)) {
                to_commit.push_back(path);
                continue;
            }

            auto content = read_file_bytes(full_path);
            Blob blob(std::move(content));
            auto current_id = Hash::compute_id(ObjectType::Blob, blob.serialize());

            if (current_id != entry.blob_id) {
                not_staged.push_back(path);
            } else if (head_entries.count(path)) {
                if (head_entries[path] != entry.blob_id) {
                    to_commit.push_back(path);
                }
            } else {
                to_commit.push_back(path);
            }
        }

        for (const auto& [path, _] : head_entries) {
            if (!repo->index().has_entry(path)) {
                to_commit.push_back(path);
            }
        }

        for (const auto& file : wt_files) {
            if (!repo->index().has_entry(file.string())) {
                untracked.push_back(file.string());
            }
        }

        std::sort(to_commit.begin(), to_commit.end());
        std::sort(not_staged.begin(), not_staged.end());
        std::sort(untracked.begin(), untracked.end());

        if (!to_commit.empty()) {
            std::cout << "Changes to be committed:\n";
            for (const auto& f : to_commit) {
                std::cout << "  (use \"gitx add <file>\" to update what will be committed)\n";
                std::cout << "\tnew file:   " << f << "\n";
            }
            std::cout << "\n";
        }

        if (!not_staged.empty()) {
            std::cout << "Changes not staged for commit:\n";
            std::cout << "  (use \"gitx add <file>\" to update what will be committed)\n";
            for (const auto& f : not_staged) {
                std::cout << "\tmodified:   " << f << "\n";
            }
            std::cout << "\n";
        }

        if (!untracked.empty()) {
            std::cout << "Untracked files:\n";
            std::cout << "  (use \"gitx add <file>\" to include in what will be committed)\n";
            for (const auto& f : untracked) {
                std::cout << "\t" << f << "\n";
            }
            std::cout << "\n";
        }

        if (to_commit.empty() && not_staged.empty() && untracked.empty()) {
            std::cout << "nothing to commit, working tree clean\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace gitx
