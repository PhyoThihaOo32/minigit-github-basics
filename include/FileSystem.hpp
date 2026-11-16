#pragma once
#include <filesystem>
#include <string>

namespace minigit {

    struct WriteResult {
        bool ok = false;
        std::string error;
    };
    struct ReadResult {
        bool ok = false;
        std::string text;
        std::string error;
    };

    // General filesystem operations share this stateless class.
    class FileSystem {
    public:
        FileSystem();

        WriteResult writeTextFile(const std::filesystem::path& filePath,const std::string& text) const;
        
        ReadResult readTextFile(const std::filesystem::path& filePath) const;
    };

} // namespace minigit
