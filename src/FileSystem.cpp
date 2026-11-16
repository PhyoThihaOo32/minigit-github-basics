#include "FileSystem.hpp"

namespace minigit {

FileSystem::FileSystem() = default;

void FileSystem::copy(const std::filesystem::path& source, const std::filesystem::path& target) const {
    std::filesystem::copy_file(source, target);
}

} // namespace minigit
