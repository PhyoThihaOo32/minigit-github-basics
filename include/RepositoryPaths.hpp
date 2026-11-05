#pragma once
#include <string>
#include <filesystem>

namespace minigit {

// Store repository locations without creating files or directories.
class RepositoryPaths {
public:
    explicit RepositoryPaths(std::filesystem::path projectRoot);

    const std::filesystem::path& projectRoot() const;
    const std::filesystem::path& metadataRoot() const;
    std::filesystem::path stageFile(const std::string& path) const;

private:
    std::filesystem::path projectRoot_;
    std::filesystem::path metadataRoot_;
};

} // namespace minigit
