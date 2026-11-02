#include "PathLab.hpp"
#include <fstream>
#include <system_error>

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
} // namespace minigit

