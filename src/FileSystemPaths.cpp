#include "FileSystem.hpp"

#include <system_error>

namespace minigit
{

std::filesystem::file_status FileSystem::fileStatus(
    const std::filesystem::path& path) const
{
    std::error_code error;
    const auto status = std::filesystem::symlink_status(path, error);

    // init needs to distinguish missing metadata from an inspection failure.
    if (error == std::errc::no_such_file_or_directory)
    {
        return std::filesystem::file_status(std::filesystem::file_type::not_found);
    }
    if (error)
    {
        throw std::filesystem::filesystem_error("could not inspect path", path, error);
    }
    return status;
}

void FileSystem::createDirectories(const std::filesystem::path& path) const
{
    // The throwing overload reports failures and accepts an existing directory.
    std::filesystem::create_directories(path);
}

} // namespace minigit
