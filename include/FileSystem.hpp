#pragma once
#include <filesystem>

namespace minigit {

// General filesystem operations share this stateless class.
class FileSystem {
public:
    FileSystem();


    // Copy a regular file without overwriting an existing file.
    void copy(const std::filesystem::path& source,const std::filesystem::path& target) const;
};

} // namespace minigit
