#include "gitx/commands/commands.hpp"
#include "gitx/repository.hpp"
#include "gitx/tree.hpp"
#include <iostream>
#include <map>
#include <filesystem>

namespace gitx {

// Build tree recursively from index entries under a directory
ObjectId build_tree(Repository& repo, const std::string& prefix) {
    std::map<std::string, TreeEntry> entries;
    auto sorted_paths = repo.index().sorted_paths();

    for (const auto& path : sorted_paths) {
        // Check if this path is under our prefix
        std::string relative = path;
        if (!prefix.empty()) {
            if (path.substr(0, prefix.size()) != prefix) continue;
            relative = path.substr(prefix.size());
            if (!relative.empty() && relative[0] == '/') {
                relative = relative.substr(1);
            }
        }

        if (relative.empty()) continue;

        // Find if this is a direct child or needs recursion
        auto slash_pos = relative.find('/');
        if (slash_pos == std::string::npos) {
            // Direct child file
            const auto& entry = repo.index().get_entry(path);
            entries[relative] = TreeEntry{ObjectType::Blob, relative, entry.blob_id};
        } else {
            // Directory - recurse
            std::string dir_name = relative.substr(0, slash_pos);
            std::string sub_prefix = prefix.empty() ? dir_name : prefix + "/" + dir_name;
            if (entries.find(dir_name) == entries.end()) {
                ObjectId tree_id = build_tree(repo, sub_prefix);
                entries[dir_name] = TreeEntry{ObjectType::Tree, dir_name, tree_id};
            }
        }
    }

    // Build tree object
    std::vector<TreeEntry> tree_entries;
    for (auto& [_, entry] : entries) {
        tree_entries.push_back(std::move(entry));
    }
    Tree tree(std::move(tree_entries));
    return repo.object_store().write(ObjectType::Tree, tree.serialize());
}

int cmd_write_tree(const std::vector<std::string>& args) {
    try {
        auto repo = Repository::find();
        if (!repo) {
            std::cerr << "fatal: not a gitx repository\n";
            return 1;
        }

        repo->load_index();
        if (repo->index().empty()) {
            std::cerr << "error: nothing to commit (empty index)\n";
            return 1;
        }

        ObjectId tree_id = build_tree(*repo, "");
        std::cout << tree_id << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace gitx
