#pragma once
#include <filesystem>
#include <string>

namespace minigit {

// Build locations only; neither helper creates files or directories.
std::filesystem::path metadataDirectory(const std::filesystem::path& root);
std::filesystem::path headPath(const std::filesystem::path& root);

// Keep Ariane's existing name compatible with the headPath API already on main.
std::filesystem::path headFile(const std::filesystem::path& root);

// Preserve the Issue 3 bool interface; both writers use the same implementation.
bool writeToFile(const std::filesystem::path& filePath, const std::string& text);

struct WriteResult {
    bool ok = false;
    std::string error;
};

// Create missing parent directories and replace the file's existing contents.
// The caller supplies the finished path; failures include a useful message.
WriteResult writeTextFile(const std::filesystem::path& filePath,
                          const std::string& text);

struct ReadResult {
    bool ok = false;
    std::string text;
    std::string error;
};

// Read the complete file. Filesystem and I/O failures return ok=false, an empty
// text string, and an explanation. Empty files are successful reads.
ReadResult readTextFile(const std::filesystem::path& filePath);

} // namespace minigit
