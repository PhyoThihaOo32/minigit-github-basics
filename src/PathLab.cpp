#include "PathLab.hpp"
#include <string>
#include <fstream>

namespace minigit {
// Implement the focused behavior described by the active Issue.
std::filesystem::path getMetadataPath(const std::filesystem::path& root) {
    return root / ".minigit";
}
// Add one helper that returns the `HEAD` file path inside `.minigit`.
std::filesystem::path getHeadPath(const std::filesystem::path& root) {
    return metadataDirectory(root) / "HEAD";
}
//Add a small filesystem helper that creates the parent directory of a requested file path, then writes the supplied text.
void writeFile(const std::filesystem::path& filePath, const std::string& text) {
    std::filesystem::create_directories(filePath.parent_path());
    std::ofstream outFile(filePath);
    outFile << text;
}
}