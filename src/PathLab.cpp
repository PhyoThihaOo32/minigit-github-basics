#include "PathLab.hpp"
#include <fstream>
#include <system_error>
#include <sstream>

namespace minigit {

        std::filesystem::path metadataDirectory(const std::filesystem::path &root)
    {
        return root / ".minigit";
    }
// Implement the focused behavior described by the active Issue.

// Issue 2
std::filesystem::path headFile(const std::filesystem::path& root) {
    return metadataDirectory(root) / "HEAD";
}

// Issue 3
WriteResult writeTextFile(const std::filesystem::path& filePath, const std::string& text) {
    const std::filesystem::path parent = filePath.parent_path();

    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            return {false, "could not create directory '" + parent.string() + "': " + ec.message()};
        }
    }

    std::ofstream out(filePath, std::ios::binary | std::ios::trunc);
    if (!out) {
        return {false, "could not open '" + filePath.string() + "' for writing"};
    }

    out << text;
    out.close();
    if (!out) {
        return {false, "could not write to '" + filePath.string() + "'"};
    }

    return {true, ""};
}

// Issue 4
ReadResult readTextFile(const std::filesystem::path& filePath) {
    std::error_code ec;

    if (!std::filesystem::exists(filePath, ec)) {
        return {false, "", "file does not exist: '" + filePath.string() + "'"};
    }
    if (!std::filesystem::is_regular_file(filePath, ec)) {
        return {false, "", "not a regular file: '" + filePath.string() + "'"};
    }

    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        return {false, "", "could not open '" + filePath.string() + "' for reading"};
    }

    std::ostringstream contents;
    contents << in.rdbuf();
    if (in.bad()) {
        return {false, "", "could not read '" + filePath.string() + "'"};
    }

    return {true, contents.str(), ""};
}
} // namespace minigit

