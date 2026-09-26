#include "gitx/commands/commands.hpp"
#include "gitx/repository.hpp"
#include "gitx/hash.hpp"
#include "gitx/blob.hpp"
#include "gitx/utils/file_utils.hpp"
#include <iostream>
#include <filesystem>

namespace gitx {

int cmd_add(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cerr << "Usage: gitx add <path>\n";
        return 1;
    }

    try {
        auto repo = Repository::find();
        if (!repo) {
            std::cerr << "fatal: not a gitx repository\n";
            return 1;
        }

        repo->load_index();
        auto& index = repo->index();

        auto target = std::filesystem::path(args[2]);
        if (!std::filesystem::exists(target)) {
            std::cerr << "fatal: pathspec '" << args[2] << "' did not match any files\n";
            return 1;
        }

        std::vector<std::string> files_to_add;

        if (std::filesystem::is_regular_file(target)) {
            // Check it's not inside .gitx/
            auto rel = std::filesystem::relative(target, repo->root());
            std::string rel_str = rel.string();
            for (auto& c : rel_str) { if (c == '\\') c = '/'; }
            if (rel_str.find(".gitx/") == 0 || rel_str.find(".gitx\\") == 0) {
                return 0;
            }
            files_to_add.push_back(rel_str);
        } else if (std::filesystem::is_directory(target)) {
            auto all_files = list_files(repo->root());
            auto target_rel = std::filesystem::relative(target, repo->root());
            for (const auto& f : all_files) {
                auto f_rel = std::filesystem::relative(
                    repo->root() / f, repo->root());
                // Check if this file is under target
                std::string f_str = f_rel.string();
                std::string t_str = target_rel.string();
                for (auto& c : f_str) { if (c == '\\') c = '/'; }
                for (auto& c : t_str) { if (c == '\\') c = '/'; }
                if (f_str.substr(0, t_str.size()) == t_str) {
                    files_to_add.push_back(f_str);
                }
            }
        }

        for (const auto& file : files_to_add) {
            auto full_path = repo->root() / file;
            auto content = read_file_bytes(full_path);
            Blob blob(std::move(content));
            auto id = repo->object_store().write(ObjectType::Blob, blob.serialize());

            auto file_size = static_cast<uint32_t>(std::filesystem::file_size(full_path));
            auto mtime = static_cast<uint64_t>(
                std::filesystem::last_write_time(full_path).time_since_epoch().count());

            index.add_entry(file, IndexEntry{id, file_size, mtime});
        }

        repo->save_index();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace gitx
