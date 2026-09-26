#include "gitx/commands/commands.hpp"
#include "gitx/repository.hpp"
#include "gitx/commit.hpp"
#include "gitx/tree.hpp"
#include "gitx/utils/time_utils.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>

namespace gitx {

int cmd_commit(const std::vector<std::string>& args) {
    // Parse -m flag
    std::string message;
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "-m" && i + 1 < args.size()) {
            message = args[i + 1];
            break;
        }
    }

    if (message.empty()) {
        std::cerr << "error: missing commit message\n";
        std::cerr << "Usage: gitx commit -m \"<message>\"\n";
        return 1;
    }

    try {
        auto repo = Repository::find();
        if (!repo) {
            std::cerr << "fatal: not a gitx repository\n";
            return 1;
        }

        repo->load_index();
        if (repo->index().empty()) {
            std::cerr << "nothing to commit\n";
            return 1;
        }

        // Build tree from index
        std::string tree_id = [&]() -> std::string {
            // Use write-tree logic inline
            std::map<std::string, TreeEntry> entries;
            auto sorted_paths = repo->index().sorted_paths();

            for (const auto& path : sorted_paths) {
                auto slash_pos = path.find('/');
                if (slash_pos == std::string::npos) {
                    const auto& entry = repo->index().get_entry(path);
                    entries[path] = TreeEntry{ObjectType::Blob, path, entry.blob_id};
                } else {
                    std::string dir_name = path.substr(0, slash_pos);
                    if (entries.find(dir_name) == entries.end()) {
                        // Build sub-tree
                        std::vector<TreeEntry> sub_entries;
                        for (const auto& p : sorted_paths) {
                            if (p.substr(0, dir_name.size()) == dir_name &&
                                p.size() > dir_name.size() && p[dir_name.size()] == '/') {
                                std::string rel = p.substr(dir_name.size() + 1);
                                if (rel.find('/') == std::string::npos) {
                                    const auto& entry = repo->index().get_entry(p);
                                    sub_entries.push_back(
                                        TreeEntry{ObjectType::Blob, rel, entry.blob_id});
                                }
                            }
                        }
                        Tree sub_tree(std::move(sub_entries));
                        auto sub_id = repo->object_store().write(
                            ObjectType::Tree, sub_tree.serialize());
                        entries[dir_name] = TreeEntry{ObjectType::Tree, dir_name, sub_id};
                    }
                }
            }

            std::vector<TreeEntry> tree_entries;
            for (auto& [_, entry] : entries) {
                tree_entries.push_back(std::move(entry));
            }
            Tree tree(std::move(tree_entries));
            return repo->object_store().write(ObjectType::Tree, tree.serialize());
        }();

        // Read HEAD as parent
        auto parent = repo->read_head();

        // Create commit
        std::string author = "GitX User <user@gitx.local>";
        std::string timestamp = current_timestamp_iso8601();

        Commit commit(tree_id, parent, author, timestamp, message);
        auto commit_id = repo->object_store().write(ObjectType::Commit, commit.serialize());

        // Update HEAD
        repo->write_head(commit_id);

        std::cout << "[" << (parent ? "main" : "main (root-commit)") << " "
                  << commit_id.substr(0, 7) << "] " << message << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace gitx
