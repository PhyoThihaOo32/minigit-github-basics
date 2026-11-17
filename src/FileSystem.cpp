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

    // FS - 6: Update Head file with new commit ID. Return true if successful, false otherwise.
bool FileSystem::updateHEAD(std::filesystem::path& headpath,
                                std::string& newID) const
    {
        std::error_code error;
        if (std::filesystem::is_directory(headpath, error)) {
            return false;
        }

        // writeTextFile truncates, so the old ID is fully replaced.
        return writeTextFile(headpath, newID).ok;
    }

    // FS-6: Remove the staged copies that were saved in a checkpoint.
    bool FileSystem::clearStage(std::vector<std::string>& selectedPaths,
                                std::filesystem::path& stagePath,
                                std::filesystem::path& indexPath) const
    {
        namespace fs = std::filesystem;
        std::error_code error;

        if (!fs::is_directory(stagePath, error) || error) {
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

        // Validate every path before deleting anything, so a bad entry doesn't leave the stage in a half-cleared state.
        std::vector<fs::path> targets;
        for (const auto& selected : selectedPaths) {
            if (selected.empty()) {
                return false;
            }

            const fs::path target = fs::weakly_canonical(stageRoot / selected, error);
            if (error) {
                return false;
            }

            // The target must sit strictly inside the stage directory.
            const fs::path relative = target.lexically_relative(stageRoot);
            if (relative.empty() || relative == "." ||
                *relative.begin() == "..") {
                return false;
            }

            // Never delete the index, even if it lives inside the stage.
            if (target == indexFile) {
                return false;
            }

            targets.push_back(target);
        }

        for (const auto& target : targets) {
            fs::remove_all(target, error);
            if (error) {
                return false;
            }
        }
        return true;
} // namespace minigit
