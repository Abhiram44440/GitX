#include "gitx/commands/commands.hpp"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    if (args.size() < 2) {
        gitx::cmd_help(args);
        return 0;
    }

    const std::string& command = args[1];

    if (command == "init") return gitx::cmd_init(args);
    if (command == "hash-object") return gitx::cmd_hash_object(args);
    if (command == "cat-object") return gitx::cmd_cat_object(args);
    if (command == "add") return gitx::cmd_add(args);
    if (command == "status") return gitx::cmd_status(args);
    if (command == "write-tree") return gitx::cmd_write_tree(args);
    if (command == "commit") return gitx::cmd_commit(args);
    if (command == "log") return gitx::cmd_log(args);
    if (command == "revert") return gitx::cmd_revert(args);
    if (command == "help") return gitx::cmd_help(args);

    std::cerr << "Unknown command: " << command << "\n";
    std::cerr << "Run 'gitx help' for available commands.\n";
    return 1;
}
