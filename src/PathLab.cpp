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
//write to file
bool writeToFile(const std::filesystem::path& filePath, const std::string& text) {
    
    try{
        std::filesystem::path parent = filePath.parent_path();
        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }
    }
    catch (const std::filesystem::filesystem_error&) {
        return false;
    }
    
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    file << text;
    file.close();
    return !file.fail();
}
} // namespace minigit
