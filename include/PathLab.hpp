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

// Write text to a file and create any missing parent directories.
bool writeToFile(const std::filesystem::path& filePath, const std::string& text);

// Read a text file back into a string and report failure for missing or unreadable file.
bool readTextFile(const std::filesystem::path& filePath, std::string& text);
bool readFromFile(const std::filesystem::path& filePath, std::string& text);
bool readText(const std::filesystem::path& filePath, std::string& text);

// Compatibility alias for the earlier starter implementation used in the project.
std::filesystem::path getMetadataPath(const std::filesystem::path& root);
std::filesystem::path metadataPath(const std::filesystem::path& root);

} // namespace minigit
