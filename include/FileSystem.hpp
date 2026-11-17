#pragma once
#include <filesystem>
#include <string>
#include <vector>

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

        // Read one index entry per line. Throw std::runtime_error if reading fails.
        std::vector<std::string> readIndex(std::filesystem::path& indexPath) const;

        WriteResult writeTextFile(const std::filesystem::path& filePath,const std::string& text) const;
        ReadResult readTextFile(const std::filesystem::path& filePath) const;

        void copy(const std::filesystem::path& source,const std::filesystem::path& target) const;

        // Delete a file or directory. If the path is a directory, delete it recursively.
        bool updateHEAD(std::filesystem::path& headpath,std::string& newID) const;
        bool clearStage(std::vector<std::string>& selectedPaths,std::filesystem::path& stagePath, std::filesystem::path& indexPath) const;
    };
} // namespace minigit
