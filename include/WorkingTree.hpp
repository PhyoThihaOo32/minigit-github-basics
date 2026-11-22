#pragma once

#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"

#include <string>
#include <vector>

namespace minigit
{

class WorkingTree
{
public:
    // Both dependencies must outlive this object.
    WorkingTree(const RepositoryPaths& paths, const FileSystem& fileSystem);

    // Return project-relative file names using '/', excluding .git and .minigit.
    std::vector<std::string> listFiles() const;

private:
    const RepositoryPaths& paths_;
    const FileSystem& fileSystem_;
};

} // namespace minigit
