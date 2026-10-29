#pragma once
#include <filesystem>

namespace minigit
{

    // Return the path to the metadata folder inside a repository.
    // This builds a path only; it does not create the folder.
    std::filesystem::path metadataDirectory(const std::filesystem::path &root);
} // namespace minigit