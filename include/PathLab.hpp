#pragma once
#include <filesystem>

namespace minigit
{

    // Return the path to the metadata folder inside a repository.
    // This builds a path only; it does not create the folder.
    std::filesystem::path metadataDirectory(const std::filesystem::path &root);

    // Return the HEAD file path without creating files or directories.
    std::filesystem::path headPath(const std::filesystem::path &root);

} // namespace minigit