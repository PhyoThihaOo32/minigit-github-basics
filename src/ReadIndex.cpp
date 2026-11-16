#include "FileSystem.hpp"

#include <sstream>
#include <stdexcept>

namespace minigit {

    std::vector<std::string> FileSystem::readIndex(std::filesystem::path& indexPath) const {
        const auto result = readTextFile(indexPath);
        if (!result.ok) {
            throw std::runtime_error(result.error);
        }

        std::istringstream lines(result.text);
        std::vector<std::string> entries;
        std::string entry;
        while (std::getline(lines, entry)) {
            // Strip the CR in a CRLF line ending, preserving the rest of the entry.
            if (!lines.eof() && !entry.empty() && entry.back() == '\r') {
                entry.pop_back();
            }
            entries.push_back(entry);
        }
        return entries;
    }

} // namespace minigit
