#pragma once
#include <filesystem>
#include <string>

namespace minigit
{

    // Return the path to the metadata folder inside a repository.
    // This builds a path only; it does not create the folder.
    std::filesystem::path metadataDirectory(const std::filesystem::path &root);

    // Return the HEAD file path without creating files or directories.
    std::filesystem::path headPath(const std::filesystem::path &root);

    // Write text to the caller's path, creating missing parent directories.
    // Replace existing file contents. Return false if directory creation,
    // opening, writing, or closing fails; otherwise return true.
    bool writeText(const std::filesystem::path &filePath,
                   const std::string &text);

    // Return true and fill text on success. On failure, clear text and explain
    // the problem in errorMessage. Successful reads clear any previous error.
    bool readText(const std::filesystem::path &filePath,
                  std::string &text,
                  std::string &errorMessage);

} // namespace minigit
