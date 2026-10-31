#pragma once
#include <filesystem>

namespace minigit {

// Week 1 Issues will add small helpers for building paths below a repository root.
// Path construction describes locations; it does not create files or directories.
// Return the path to the metadata folder inside a repository.
// This builds a path only; it does not create the folder.
std::filesystem::path metadataDirectory(const std::filesystem::path& root);
} // namespace minigit
