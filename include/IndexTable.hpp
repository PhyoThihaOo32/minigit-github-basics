#pragma once

#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace minigit
{

// Manage selected project-relative paths in the root .minigit/index.txt.
class IndexTable
{
public:
    // Copy the index location. fileSystem must outlive this object.
    IndexTable(const RepositoryPaths& paths, const FileSystem& fileSystem);

    // Copy indexPath_ to a local path before calling FileSystem::readIndex(),
    // whose existing API takes a non-const reference. Keep this method const.
    std::vector<std::string> readIndex() const;

    // The caller validates paths first; reject CR/LF here before writing entries.
    // Adding an existing entry changes nothing. removePath only unselects a path.
    void addPath(const std::string& path) const;
    void removePath(const std::string& path) const;

    // Empty the index while keeping the file itself.
    void clearIndex() const;

private:
    std::filesystem::path indexPath_;
    const FileSystem& fileSystem_;
};

} // namespace minigit
