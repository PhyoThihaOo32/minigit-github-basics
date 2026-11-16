#pragma once
#include <filesystem>
#include <string>

namespace minigit {

// Build locations only; neither helper creates files or directories.
std::filesystem::path metadataDirectory(const std::filesystem::path& root);
std::filesystem::path headPath(const std::filesystem::path& root);
} // namespace minigit
