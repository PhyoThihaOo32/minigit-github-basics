#include "PathLab.hpp"
#include <string>
#include <fstream>

namespace minigit {
// Implement the focused behavior described by the active Issue.
std::filesystem::path metadataDirectory(const std::filesystem::path& root) {
    return root / ".minigit";
}
//head path
std::filesystem::path headPath(const std::filesystem::path& root) {
    return metadataDirectory(root) / "HEAD";
}
//Add a small filesystem helper that creates the parent directory of a requested file path, then writes the supplied text.
void writeTextFile(const std::filesystem::path& filePath, const std::string& content) {
    std::filesystem::create_directories(filePath.parent_path());
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filePath.string());
    }
    file << content;
    file.close();
}

//Add a small helper to read a text file and report a useful failure when the path does not exist or cannot be read.
std::string readTextFile(const std::filesystem::path& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath.string());
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

} // namespace minigit
