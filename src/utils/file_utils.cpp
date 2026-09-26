#include "gitx/utils/file_utils.hpp"
#include <fstream>
#include <stdexcept>

namespace gitx {

std::vector<std::byte> read_file_bytes(const std::filesystem::path& path) {
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if (!ifs) throw std::runtime_error("Cannot open file: " + path.string());

    auto size = ifs.tellg();
    std::vector<std::byte> data(static_cast<size_t>(size));
    ifs.seekg(0);
    ifs.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

std::string read_file_text(const std::filesystem::path& path) {
    std::ifstream ifs(path);
    if (!ifs) throw std::runtime_error("Cannot open file: " + path.string());
    return std::string(std::istreambuf_iterator<char>(ifs),
                       std::istreambuf_iterator<char>());
}

void write_file_bytes(const std::filesystem::path& path, const std::vector<std::byte>& data) {
    auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) throw std::runtime_error("Cannot write file: " + path.string());
    ofs.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size()));
}

void write_file_text(const std::filesystem::path& path, const std::string& text) {
    auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }
    std::ofstream ofs(path);
    if (!ofs) throw std::runtime_error("Cannot write file: " + path.string());
    ofs << text;
}

std::vector<std::filesystem::path> list_files(const std::filesystem::path& dir) {
    std::vector<std::filesystem::path> result;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;

        auto rel = std::filesystem::relative(entry.path(), dir);
        std::string rel_str = rel.string();
        for (auto& c : rel_str) {
            if (c == '\\') c = '/';
        }
        if (rel_str.find(".gitx/") == 0 || rel_str.find(".gitx\\") == 0) continue;

        result.push_back(rel);
    }
    std::sort(result.begin(), result.end());
    return result;
}

} // namespace gitx
