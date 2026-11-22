#pragma once

#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"

#include <string>
#include <vector>

namespace minigit
{

struct CommitInfo
{
    std::string id;
    std::string parentId;
    std::string message;
    std::string time;
};

// Store complete snapshots and retrieve commit records.
class CommitStore
{
public:
    // Both dependencies must outlive this object.
    CommitStore(const RepositoryPaths& paths, const FileSystem& fileSystem);

    // Validate selected relative paths (including CR/LF rejection) before joining
    // storage paths. Save a new snapshot/record without replacing an existing ID.
    // Return its ID without changing HEAD or clearing stage/index.
    std::string saveCommit(const std::vector<std::string>& selectedPaths,
                           const std::string& message) const;

    std::vector<std::string> getHistory() const;
    // Validate IDs as positive decimal digits before constructing commit paths.
    CommitInfo getCommitInfo(const std::string& commitId) const;
    std::vector<std::string> getCommittedFiles(const std::string& commitId) const;

private:
    const RepositoryPaths& paths_;
    const FileSystem& fileSystem_;
};

} // namespace minigit
