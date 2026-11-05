#pragma once

#include <filesystem>

namespace minigit {

// Store repository locations without creating files or directories.
class RepositoryPaths {
public:
    explicit RepositoryPaths(std::filesystem::path projectRoot);

    const std::filesystem::path& projectRoot() const;
    const std::filesystem::path& metadataRoot() const;

    //Returns path to HEAD file to track latest commit
    std::filesystem::path headFile() const;
    //Returns path to index file to track staged files
    std::filesystem::path indexFile() const;

private:
    std::filesystem::path projectRoot_;
    std::filesystem::path metadataRoot_;
};

} // namespace minigit
