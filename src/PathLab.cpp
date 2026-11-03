#include "PathLab.hpp"

#include <fstream>
#include <string>

namespace minigit {

std::filesystem::path metadataDirectory(const std::filesystem::path& root) {
    return root / ".minigit";
}

std::filesystem::path headPath(const std::filesystem::path& root) {
    return metadataDirectory(root) / "HEAD";
}

bool writeToFile(const std::filesystem::path& filePath, const std::string& text) {
    if (filePath.empty()) {
        return false;
    }

    const auto parent = filePath.parent_path();
    std::error_code ec;
    if (!parent.empty() && !std::filesystem::exists(parent, ec)) {
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            return false;
        }
    }

    std::ofstream output(filePath, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        return false;
    }

    output << text;
    return output.good();
}

bool readTextFile(const std::filesystem::path& filePath, std::string& text) {
    text.clear();
    std::ifstream input(filePath, std::ios::binary);
    if (!input.is_open()) {
        return false;
    }

    std::string buffer;
    char ch = 0;
    while (input.get(ch)) {
        buffer.push_back(ch);
    }

    if (input.bad()) {
        return false;
    }

    text = std::move(buffer);
    return true;
}

bool readFromFile(const std::filesystem::path& filePath, std::string& text) {
    return readTextFile(filePath, text);
}

bool readText(const std::filesystem::path& filePath, std::string& text) {
    return readTextFile(filePath, text);
}

std::filesystem::path getMetadataPath(const std::filesystem::path& root) {
    return metadataDirectory(root);
}

std::filesystem::path metadataPath(const std::filesystem::path& root) {
    return metadataDirectory(root);
}

} // namespace minigit

