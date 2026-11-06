#pragma once

#include <filesystem>
#include <string>

namespace minigit
{

    // Store repository locations without creating files or directories.
    class RepositoryPaths
    {
    public:
        explicit RepositoryPaths(std::filesystem::path projectRoot);

        const std::filesystem::path &projectRoot() const;
        const std::filesystem::path &metadataRoot() const;
        std::filesystem::path stageFile(const std::string &path) const;

        // Normalize a relative file path or throw if its text is unsafe.
        std::string safeRelativeFile(const std::string &argument) const;

    private:
        std::filesystem::path projectRoot_;
        std::filesystem::path metadataRoot_;
    };

} // namespace minigit
