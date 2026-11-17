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

        // FS-6: Update Head file with new commit ID. Return true if successful, false otherwise.
        bool updateHEAD(std::filesystem::path& headpath,std::string& newID) const;
        // FS-6: Clear the stage of selected files. Return true if successful, false otherwise.
        bool clearStage(std::vector<std::string>& selectedPaths,std::filesystem::path& stagePath, std::filesystem::path& indexPath) const;

        // Each paths[i] is the filesystem location for selectedPaths[i].
        // Results are replaced with one present/missing/error status per selection.
        void inespectFiles(std::vector<std::filesystem::path>& paths,
                           std::vector<std::string>& selectedPaths,
                           std::vector<std::string>& results);

        // Here paths contains staged file locations in the same order as selectedPaths.
        // Returns false for an empty selection, a missing/non-file entry, or an inspection error.
        bool checkStage(std::vector<std::filesystem::path>& paths,
                        std::vector<std::string>& selectedPaths);
        
    };
} // namespace minigit
