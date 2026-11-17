#include "FileSystem.hpp"
#include "TextReader.hpp"
#include <algorithm>
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

    // Replace HEAD after the new ID has been written successfully.
    bool FileSystem::updateHEAD(std::filesystem::path& headpath, std::string& newID) const
    {
        namespace fs = std::filesystem;
        std::error_code error;

        if (headpath.empty() || newID.empty()) {
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

        const auto parent = headpath.has_parent_path() ? headpath.parent_path() : fs::path(".");
        fs::create_directories(parent, error);
        if (error) {
            return false;
        }

        // Claim a directory beside HEAD so the replacement stays on the same filesystem.
        for (int attempt = 0; attempt < 100; ++attempt) {
            const auto scratch = parent / (".minigit-head-update-" + std::to_string(attempt));
            if (!fs::create_directory(scratch, error)) {
                if (error && error != std::errc::file_exists) {
                    return false;
                }
                continue;
            }

            const auto replacement = scratch / "replacement";
            const bool written = writeTextFile(replacement, newID).ok;
            if (written) {
                fs::rename(replacement, headpath, error);
            }
            const bool updated = written && !error;

            std::error_code cleanupError;
            if (!updated) {
                fs::remove(replacement, cleanupError);
            }
            fs::remove(scratch, cleanupError);
            return updated;
        }
        return false;
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

        if (std::find(targets.begin(), targets.end(), current) == targets.end()) {
                targets.push_back(current);
            }
        }
        
        //Delete files after validation
        for (const auto& target : targets) {
            if (!fs::remove(target, error) || error) {
                return false;
            }
        }

        return true;
    }
    void FileSystem::inespectFiles(std::vector<std::filesystem::path>& paths,
                                   std::vector<std::string>& selectedPaths,
                                   std::vector<std::string>& results) {
        results.clear();
        results.reserve(selectedPaths.size());

        for (std::size_t i = 0; i < selectedPaths.size(); ++i) {
            if (i >= paths.size()) {
                results.push_back("missing: " + selectedPaths[i]);
                continue;
            }

            std::error_code error;
            const auto status = std::filesystem::status(paths[i], error);
            if (error == std::errc::no_such_file_or_directory
                || (!error && !std::filesystem::exists(status))) {
                results.push_back("missing: " + selectedPaths[i]);
            } else if (!error && std::filesystem::is_regular_file(status)) {
                results.push_back("present: " + selectedPaths[i]);
            } else {
                results.push_back("could not inspect: " + selectedPaths[i]);
            }
        }
    }

    bool FileSystem::checkStage(std::vector<std::filesystem::path>& paths,
                                std::vector<std::string>& selectedPaths) {
        if (selectedPaths.empty() || paths.size() < selectedPaths.size()) {
            return false;
        }

        for (std::size_t i = 0; i < selectedPaths.size(); ++i) {
            namespace fs = std::filesystem;
            const fs::path relative = fs::path(selectedPaths[i]).lexically_normal();
            if (relative.empty() || relative == "." || relative.has_root_path()
                || *relative.begin() == "..") {
                return false;
            }

            const fs::path stagedFile = paths[i].lexically_normal();
            fs::path stageRoot = stagedFile;
            for ([[maybe_unused]] const auto& part : relative) {
                stageRoot = stageRoot.parent_path();
            }
            if ((stageRoot / relative).lexically_normal() != stagedFile) {
                return false;
            }

            std::error_code error;
            const auto rootStatus = fs::symlink_status(stageRoot, error);
            if (error || fs::is_symlink(rootStatus) || !fs::is_directory(rootStatus)) {
                return false;
            }

            // Match clearStage: walk each selected component without following links.
            fs::path current = stageRoot;
            for (const auto& part : relative) {
                current /= part;
                const auto status = fs::symlink_status(current, error);
                if (error || fs::is_symlink(status)) {
                    return false;
                }

                const bool isFinalPart = current == stagedFile;
                if (isFinalPart ? !fs::is_regular_file(status) : !fs::is_directory(status)) {
                    return false;
                }
            }
        }
        return true;
    }
}// namespace minigit
