#include "PathLab.hpp"

namespace minigit {

        std::filesystem::path metadataDirectory(const std::filesystem::path &root)
    {
        return root / ".minigit";
    }
// Implement the focused behavior described by the active Issue.

// Issue 2
std::filesystem::path headFile(const std::filesystem::path& root) {
    return metadataDirectory(root) / "HEAD";
}
} // namespace minigit

