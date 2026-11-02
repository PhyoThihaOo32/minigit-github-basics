#pragma once
#include <filesystem>
#include <string>

namespace minigit {

// Week 1 Issues will add small helpers for building paths below a repository root.
// Path construction describes locations; it does not create files or directories.

// Return the path to the metadata folder inside a repository.
// This builds a path only; it does not create the folder.
std::filesystem::path metadataDirectory(const std::filesystem::path& root);

// Issue 2
// Return the path to the HEAD file inside the metadata folder.
// This builds a path only; it does not create or write the file.
std::filesystem::path headFile(const std::filesystem::path& root);

// Issue 3
// Result of a filesystem write: ok is true on success; on failure, error says why.
struct WriteResult {
    bool ok = false;
    std::string error;
};

// Create the parent directory of filePath if it is missing, then write text to the file
// (replacing any existing contents). Takes a finished path; it does not build paths.
WriteResult writeTextFile(const std::filesystem::path& filePath, const std::string& text);

} // namespace minigit
