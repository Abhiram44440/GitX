#include "gitx/commands/commands.hpp"
#include "gitx/repository.hpp"
#include "gitx/commit.hpp"
#include <iostream>
#include <sstream>

namespace gitx {

int cmd_log(const std::vector<std::string>& args) {
    try {
        auto repo = Repository::find();
        if (!repo) {
            std::cerr << "fatal: not a gitx repository\n";
            return 1;
        }

        auto head = repo->read_head();
        if (!head) {
            std::cout << "No commits yet.\n";
            return 0;
        }

        std::string current = *head;
        while (!current.empty()) {
            auto [type, data] = repo->object_store().read(current);
            std::string text = from_bytes(data);

            // Parse commit
            std::string tree_id, parent_id, author, timestamp, message;
            std::istringstream iss(text);
            std::string line;
            bool in_message = false;

            while (std::getline(iss, line)) {
                if (in_message) {
                    message += line + "\n";
                    continue;
                }
                if (line.empty()) {
                    in_message = true;
                    continue;
                }
                if (line.substr(0, 5) == "tree ") {
                    tree_id = line.substr(5);
                } else if (line.substr(0, 7) == "parent ") {
                    parent_id = line.substr(7);
                } else if (line.substr(0, 7) == "author ") {
                    author = line.substr(7);
                } else if (line.substr(0, 10) == "timestamp ") {
                    timestamp = line.substr(10);
                }
            }

            // Trim trailing newline from message
            while (!message.empty() && message.back() == '\n') {
                message.pop_back();
            }

            std::cout << "commit " << current << "\n";
            std::cout << "Author: " << author << "\n";
            std::cout << "Date:   " << timestamp << "\n";
            std::cout << "\n    " << message << "\n\n";

            current = parent_id;
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace gitx
