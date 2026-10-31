#include "PathLab.hpp"

namespace minigit {
// Implement the focused behavior described by the active Issue.
std::filesystem::path metadataDirectory(const std::filesystem::path& root) {
    return root / ".minigit";
}
} // namespace minigit
