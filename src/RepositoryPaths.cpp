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

std::filesystem::path RepositoryPaths::stageFile(const std::string& path) const {
    return metadataRoot_ / "stage" / path;

}


std::filesystem::path RepositoryPaths::commitDirectory(const std::string& id) const {
    return metadataRoot_ / "commits" / id;
}
 

std::filesystem::path RepositoryPaths::committedFile(const std::string& id,
                                                     const std::string& path) const {
    return commitDirectory(id) / "files" / path;
}
 

 // namespace minigit
}