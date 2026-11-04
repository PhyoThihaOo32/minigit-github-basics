#include "PathLab.hpp"
#include "TextReader.hpp"
#include <fstream>
#include <system_error>

namespace minigit {

std::filesystem::path metadataDirectory(const std::filesystem::path& root) {
    return root / ".minigit";
}


WriteResult writeTextFile(const std::filesystem::path& filePath,
                          const std::string& text) {
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

namespace detail {

ReadResult readTextStream(std::istream& input,
                          const std::filesystem::path& sourcePath) {
    // Reading through the input stream makes its state reflect a read failure.
    // Keep partial text local so a failed result never exposes incomplete data.
    std::string text;
    char character;
    while (input.get(character)) {
        text += character;
    }
    if (input.bad() || !input.eof()) {
        return {false, "", "could not read '" + sourcePath.string() + "'"};
    }
    return {true, text, ""};
}

} // namespace detail

ReadResult readTextFile(const std::filesystem::path& filePath) {
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

} // namespace minigit
