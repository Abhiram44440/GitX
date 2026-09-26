#include "gitx/commands/commands.hpp"
#include "gitx/repository.hpp"
#include "gitx/hash.hpp"
#include "gitx/blob.hpp"
#include "gitx/utils/file_utils.hpp"
#include <iostream>

namespace gitx {

int cmd_hash_object(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cerr << "Usage: gitx hash-object <file>\n";
        return 1;
    }

    try {
        auto file_path = std::filesystem::path(args[2]);
        if (!std::filesystem::exists(file_path)) {
            std::cerr << "fatal: file not found: " << args[2] << "\n";
            return 1;
        }

        auto content = read_file_bytes(file_path);
        Blob blob(std::move(content));
        auto id = Hash::compute_id(ObjectType::Blob, blob.serialize());

        // Optionally write to object store if in a repo
        auto repo = Repository::find();
        if (repo) {
            repo->object_store().write(ObjectType::Blob, blob.serialize());
        }

        std::cout << id << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace gitx
