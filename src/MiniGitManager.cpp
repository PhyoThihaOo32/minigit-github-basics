#include "MiniGitManager.hpp"

namespace minigit
{

MiniGitManager::MiniGitManager(const RepositoryPaths& paths,
                               const FileSystem& fileSystem)
    : paths_(paths), fileSystem_(fileSystem)
{
}

} // namespace minigit
