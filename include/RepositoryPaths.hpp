#pragma once
#include <string>
#include <filesystem>

namespace minigit {


class RepositoryPaths {
public:
    explicit RepositoryPaths(std::filesystem::path projectRoot);

    const std::filesystem::path& projectRoot() const;
    const std::filesystem::path& metadataRoot() const;
    std::filesystem::path stageFile(const std::string& path) const;

    
    std::filesystem::path commitDirectory(const std::string& id) const;
    
    std::filesystem::path committedFile(const std::string& id,
                                        const std::string& path) const;
    

private:
    std::filesystem::path projectRoot_;
    std::filesystem::path metadataRoot_;
};

} // namespace minigit
