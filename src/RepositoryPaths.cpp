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

std::filesystem::path RepositoryPaths::headFile() const
{
    return metadataRoot_ / "HEAD";
}

std::filesystem::path RepositoryPaths::indexFile() const
{
    return metadataRoot_ / "index";
}

std::filesystem::path RepositoryPaths::stageFile(const std::string &path) const
{
    return metadataRoot_ / "stage" / safeRelativeFile(path);
}

std::filesystem::path RepositoryPaths::commitDirectory(const std::string &id) const
{
    return metadataRoot_ / "commits" / id;
}

std::filesystem::path RepositoryPaths::committedFile(const std::string &id,
                                                    const std::string &path) const
{
    return commitDirectory(id) / "files" / safeRelativeFile(path);
}

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

    if (normalized.begin() != normalized.end() && *normalized.begin() == "..")
    {
        throw std::runtime_error("File path must stay inside the project");
    }

    return normalized.generic_string();
}

} // namespace minigit
