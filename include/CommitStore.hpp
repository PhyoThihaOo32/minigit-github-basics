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

    // Save the snapshot/record and reserve its ID. Return the ID without changing
    // HEAD or clearing stage/index; MiniGitManager completes those steps.
    std::string saveCommit(const std::vector<std::string>& selectedPaths,
                           const std::string& message) const;

    std::vector<std::string> getHistory() const;
    CommitInfo getCommitInfo(const std::string& commitId) const;
    std::vector<std::string> getCommittedFiles(const std::string& commitId) const;

private:
    const RepositoryPaths& paths_;
    const FileSystem& fileSystem_;
};

} // namespace minigit
