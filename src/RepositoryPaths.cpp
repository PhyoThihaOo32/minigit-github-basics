#include "RepositoryPaths.hpp"
#include <stdexcept>
#include <utility>

namespace minigit
{

    RepositoryPaths::RepositoryPaths(std::filesystem::path projectRoot)
        : projectRoot_(std::move(projectRoot)),
          metadataRoot_(projectRoot_ / ".minigit")
    {
    }

    const std::filesystem::path &RepositoryPaths::projectRoot() const
    {
        return projectRoot_;
    }

    const std::filesystem::path &RepositoryPaths::metadataRoot() const
    {
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

std::filesystem::path RepositoryPaths::stageFile(const std::string& path) const {
    return metadataRoot_ / "stage" / path;
    std::string RepositoryPaths::safeRelativeFile(const std::string &argument) const
    {
        if (argument.empty())
        {
            throw std::runtime_error("File path must not be empty");
        }

        const std::filesystem::path input(argument);
        if (input.has_root_path())
        {
            throw std::runtime_error("File path must be relative to the project");
        }

        // Check before normalization so it cannot hide a forbidden folder.
        for (const auto &part : input)
        {
            if (part == ".git" || part == ".minigit")
            {
                throw std::runtime_error("File path must not contain .git or .minigit");
            }
        }

        const auto normalized = input.lexically_normal();
        if (normalized.empty() || normalized == ".")
        {
            throw std::runtime_error("File path must name a file, not the current directory");
        }

        // Internal .. parts are allowed; a remaining leading .. leaves the project.
        if (*normalized.begin() == "..")
        {
            throw std::runtime_error("File path must stay inside the project");
        }

        return normalized.generic_string();
    }

    std::filesystem::path RepositoryPaths::stageFile(const std::string &path) const
    {
        return metadataRoot_ / "stage" / path;
    }

} // namespace minigit
