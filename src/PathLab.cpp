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
}
