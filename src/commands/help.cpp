#include "gitx/commands/commands.hpp"
#include <iostream>

namespace gitx {

int cmd_help(const std::vector<std::string>& args) {
    std::cout << "GitX V1 - A Git-like local version control system\n\n";
    std::cout << "Usage: gitx <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  init              Initialize a new GitX repository\n";
    std::cout << "  hash-object       Hash file contents as a blob\n";
    std::cout << "  cat-object        Display the contents of an object\n";
    std::cout << "  add               Add file contents to the index\n";
    std::cout << "  status            Show the working tree status\n";
    std::cout << "  write-tree        Create a tree object from the index\n";
    std::cout << "  commit            Record changes to the repository\n";
    std::cout << "  log               Show commit logs\n";
    std::cout << "  revert            Restore working directory to a previous commit\n";
    std::cout << "  help              Show this help message\n";
    return 0;
}

} // namespace gitx
