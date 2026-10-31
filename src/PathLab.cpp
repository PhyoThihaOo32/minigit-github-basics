#include "PathLab.hpp"
#include <fstream>
#include <system_error>

namespace minigit
{
    std::filesystem::path metadataDirectory(const std::filesystem::path &root)
    {
        return root / ".minigit";
    }

    // Reuse the metadata path helper without creating anything on disk.
    std::filesystem::path headPath(const std::filesystem::path &root)
    {
        return metadataDirectory(root) / "HEAD";
    }

    // The caller constructs the destination path; this helper handles file I/O.
    bool writeText(const std::filesystem::path &filePath,
                   const std::string &text)
    {
        const std::filesystem::path parent = filePath.parent_path();

        // A filename such as "notes.txt" has no parent path: write it in the
        // current directory instead of trying to create an empty directory path.
        if (!parent.empty())
        {
            // Report filesystem failures through the bool result. Existing
            // parent directories are valid and do not count as an error.
            std::error_code error;
            std::filesystem::create_directories(parent, error);

            if (error)
            {
                return false;
            }
        }

        // Default output mode creates the file or replaces its old contents.
        std::ofstream output(filePath);
        if (!output.is_open())
        {
            return false;
        }

        output << text;
        // Closing flushes buffered text and can also report a write failure.
        output.close();

        // Check the stream after both writing and closing have finished.
        return !output.fail();
    }

} // namespace minigit
