#include "gitx/commands/commands.hpp"
#include "gitx/repository.hpp"
#include <iostream>

namespace gitx {

int cmd_init(const std::vector<std::string>& args) {
    try {
        std::filesystem::path path = std::filesystem::current_path();
        // args[0] is the command name, check for an actual path argument
        if (args.size() > 2) {
            path = args[2];
        }
        Repository::init(path);
        std::cout << "Initialized empty GitX repository in " << (path / ".gitx").string() << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}

} // namespace gitx
