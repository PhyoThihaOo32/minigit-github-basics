#pragma once
#include <filesystem>
#include <string>

namespace minigit {

// Week 1 Issues will add small helpers for building paths below a repository root.
// Path construction describes locations; it does not create files or directories.
// Return the path to the metadata folder inside a repository.
// This builds a path only; it does not create the folder.
std::filesystem::path metadataDirectory(const std::filesystem::path& root);
// Return the path to the HEAD file inside a repository.
std::filesystem::path headPath(const std::filesystem::path& root);
//writing into file
bool writeToFile(const std::filesystem::path& filePath, const std::string& text);
} // namespace minigit
