#include "FileSystem.hpp"
#include "TextReader.hpp"
#include <fstream>
#include <system_error>

namespace minigit {

    FileSystem::FileSystem() = default;

    WriteResult FileSystem::writeTextFile( const std::filesystem::path& filePath,const std::string& text) const {
        const std::filesystem::path parent = filePath.parent_path();

                if (!parent.empty()) {
                    std::error_code error;
                    std::filesystem::create_directories(parent, error);

                    if (error) {
                        return {false, "could not create directory '" + parent.string() + "': " + error.message()};
                    }
                }

                std::ofstream output(filePath, std::ios::binary | std::ios::trunc);

                if (!output) {
                    return {false, "could not open '" + filePath.string() + "' for writing"};
                }

                output << text;
                output.close();

                if (!output) {
                    return {false, "could not write to '" + filePath.string() + "'"};
                }

                return {true, ""};
    }
    namespace detail{
        ReadResult readTextStream(
            std::istream& input,
            const std::filesystem::path& sourcePath) {std::string text;
            char character;

            while (input.get(character)) {
                text += character;
            }

            if (input.bad() || !input.eof()) {
                return {false, "", "could not read '" + sourcePath.string() + "'"};
            }

            return {true, text, ""};
        }
    }
    ReadResult FileSystem::readTextFile(const std::filesystem::path& filePath) const {

        std::error_code error;
        const auto status = std::filesystem::status(filePath, error);

        if (error) {
            return {false, "", "could not inspect '" + filePath.string() + "': " + error.message()};
        }

        if (!std::filesystem::exists(status)) {
            return {false, "", "file does not exist: '" + filePath.string() + "'"};
        }

        if (!std::filesystem::is_regular_file(status)) {
            return {false, "", "not a regular file: '" + filePath.string() + "'"};
        }

        std::ifstream input(filePath, std::ios::binary);

        if (!input) {
            return {false, "", "could not open '" + filePath.string() + "' for reading"};
        }

        return detail::readTextStream(input, filePath);
    }
    // FS-3: Copy a regular file.
    void FileSystem::copy(const std::filesystem::path& source,const std::filesystem::path& target) const
    {
        std::filesystem::copy_file(source, target);
    }

    // FS-6: Remove the staged copies that were saved in a checkpoint.
    bool FileSystem::updateHEAD(std::filesystem::path& headpath, std::string& newID) const
    {
        namespace fs = std::filesystem;
        std::error_code error;

         if (headpath.empty()) {
            return false;
        }

        const auto status = fs::symlink_status(headpath, error);
        if (error && status.type() != fs::file_type::not_found) {
            return false;
        }
        if (fs::is_symlink(status) || fs::is_directory(status)) {
            return false;
        }
        if (fs::exists(status) && !fs::is_regular_file(status)) {
            return false;
        }

        fs::path tempPath = headpath;
        tempPath += ".tmp";

        if (!writeTextFile(tempPath, newID).ok) {
            fs::remove(tempPath, error);
            return false;
        }

        fs::rename(tempPath, headpath, error);
        if (error) {
            std::error_code ignored;
            fs::remove(tempPath, ignored);
            return false;
        }

        return true;
    }
    
    bool FileSystem::clearStage(std::vector<std::string>& selectedPaths, std::filesystem::path& stagePath, std::filesystem::path& indexPath) const
    {
        namespace fs = std::filesystem;
        std::error_code error;

        // The stage itself must be a real directory, not a link to somewhere else.
        const auto stageStatus = fs::symlink_status(stagePath, error);
        if (error || !fs::is_directory(stageStatus)) {
            return false;
        }

        const fs::path stageRoot = fs::weakly_canonical(stagePath, error);
        if (error) {
            return false;
        }
        const fs::path indexFile = fs::weakly_canonical(indexPath, error);
        if (error) {
            return false;
        }

        // Validate every path before deleting anything, so a bad entry doesn't leave the stage in a partially cleared state.
        std::vector<fs::path> targets;
        for (const auto& selected : selectedPaths) {
            if (selected.empty()) {
                return false;
            }

            const fs::path relative = fs::path(selected).lexically_normal();

            // Must be a relative path that stays inside the stage.
            if (relative.has_root_path() || relative.empty() ||
                relative == "." || *relative.begin() == "..") {
                return false;
            }

            // Walk each component so no directory or file along the way is a symbolic link that could point outside the stage.
            fs::path current = stageRoot;
            for (const auto& part : relative) {
                current /= part;
                const auto partStatus = fs::symlink_status(current, error);
                if (error || fs::is_symlink(partStatus)) {
                    return false;
                }
            }

            // Only plain files may be cleared
            if (!fs::is_regular_file(fs::symlink_status(current, error)) || error) {
                return false;
            }

            // Never delete the index
            if (fs::equivalent(current, indexFile, error) || current == indexFile) {
                return false;
            }
            error.clear();

            targets.push_back(current);
        }
        //Delete files after validation
        for (const auto& target : targets) {
            if (!fs::remove(target, error) || error) {
                return false;
            }
        }

        return true;
    }
}// namespace minigit
