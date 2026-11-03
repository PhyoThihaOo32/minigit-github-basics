#pragma once

#include <filesystem>

namespace minigit {

// Return the metadata directory inside a repository root.
std::filesystem::path metadataDirectory(const std::filesystem::path& root);

// Return the HEAD file path inside the repository metadata directory.
std::filesystem::path headPath(const std::filesystem::path& root);

} // namespace minigit
