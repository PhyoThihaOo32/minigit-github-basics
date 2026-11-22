#pragma once

#include <string>
#include <vector>

namespace minigit
{

// A missing file and an existing empty file must remain distinguishable.
struct FileInfo
{
    bool exists = false;
    std::string text;
};

// Compare supplied information without accessing the filesystem.
class FileState
{
public:
    FileState();

    // selected means the path is listed in the root index.txt. Multiple results
    // allow staged content to differ again from the current working file.
    std::vector<std::string> classify(const FileInfo& working,
                                      const FileInfo& staged,
                                      const FileInfo& committed,
                                      bool selected) const;
};

} // namespace minigit
