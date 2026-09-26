#include "gitx/tree.hpp"
#include "gitx/types.hpp"

#include <iostream>
#include <string>

int main() {
    const std::string id(64, 'a');
    gitx::Tree tree({gitx::TreeEntry{gitx::ObjectType::Blob, "hello.txt", id}});

    const std::string serialized = gitx::from_bytes(tree.serialize());
    const std::string expected =
        "100644 blob hello.txt" + std::string(1, '\0') + id + "\n";

    if (serialized != expected) {
        std::cerr << "Tree entry is missing its header/object-ID separator.\n";
        return 1;
    }
    return 0;
}
