#pragma once

#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"

#include <string>

namespace minigit
{

// Coordinate commands. M2 construction uses only the completed M1 classes.
class MiniGitManager
{
public:
    // Both dependencies must outlive this object. Construction changes no files.
    MiniGitManager(const RepositoryPaths& paths, const FileSystem& fileSystem);

    // Create a new layout, preserve a valid existing repo, or throw on damage.
    // Print the success/already-initialized message; the CLI prints errors.
    void init();

    // Later milestones implement these commands; M2 must not call them.
    void add(const std::string& argument);
    void commit(const std::string& message);
    void log() const;
    void status() const;

private:
    const RepositoryPaths& paths_;
    const FileSystem& fileSystem_;
};

} // namespace minigit
