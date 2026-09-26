#include "gitx/commands/commands.hpp"
#include "gitx/repository.hpp"
#include <iostream>

namespace gitx {

int cmd_cat_object(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cerr << "Usage: gitx cat-object <id>\n";
        return 1;
    }

    try {
        auto repo = Repository::find();
        if (!repo) {
            std::cerr << "fatal: not a gitx repository\n";
            return 1;
        }

        auto [type, content] = repo->object_store().read(args[2]);
        std::cout << to_string(type) << "\n";
        // Write raw bytes as characters
        for (auto b : content) {
            std::cout.put(static_cast<char>(b));
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace gitx
