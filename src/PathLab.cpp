#include "PathLab.hpp"

namespace minigit {
// Implement the focused behavior described by the active Issue.
std::filesystem::path metadataDirectory(const std::filesystem::path& root) {
    return root / ".minigit";
}
//head path
std::filesystem::path headPath(const std::filesystem::path& root) {
    return metadataDirectory(root) / "HEAD";
}
} // namespace minigit
