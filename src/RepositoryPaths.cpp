#include "RepositoryPaths.hpp"
#include <utility>

namespace minigit {

RepositoryPaths::RepositoryPaths(std::filesystem::path projectRoot)
    : projectRoot_(std::move(projectRoot)),
      metadataRoot_(projectRoot_ / ".minigit") {
}

const std::filesystem::path& RepositoryPaths::projectRoot() const {
    return projectRoot_;
}

const std::filesystem::path& RepositoryPaths::metadataRoot() const {
    return metadataRoot_;
}

//Returns the location of HEAD Reference inside metadata
std::filesystem::path RepositoryPaths::headFile() const {
    return metadataRoot_ / "HEAD";
}
//Returns the location of index file inside metadata
std::filesystem::path RepositoryPaths::indexFile() const {
    return metadataRoot_ / "index";
}

} // namespace minigit
