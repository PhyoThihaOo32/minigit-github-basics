#include "PathLab.hpp"
#include <string>
#include <fstream>

namespace minigit {
// Implement the focused behavior described by the active Issue.
std::filesystem::path getMetadataPath(const std::filesystem::path& root) {
    return root / ".minigit";
}
}
