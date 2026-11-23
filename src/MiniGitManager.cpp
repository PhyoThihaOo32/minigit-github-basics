#include "MiniGitManager.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>


namespace minigit
{

namespace
{

namespace fs = std::filesystem;

// Metadata readers accept LF, CRLF, or no final newline (docs/storage-format.md).
std::string withoutFinalLineEnding(std::string text)
{
    if (!text.empty() && text.back() == '\n')
    {
        text.pop_back();
        if (!text.empty() && text.back() == '\r')
        {
            text.pop_back();
        }
    }
    return text;
}

// A commit ID is one or more decimal digits with no leading zero, so "1" and
// "01" can never name two different commit directories.
bool isPositiveDecimalId(const std::string& text)
{
    if (text.empty() || text.front() == '0')
    {
        return false;
    }
    for (const char character : text)
    {
        if (character < '0' || character > '9')
        {
            return false;
        }
    }
    return true;
}

// Compare canonical decimal IDs without converting them, so huge values
// cannot overflow an integer type.
bool isGreaterId(const std::string& left, const std::string& right)
{
    if (left.size() != right.size())
    {
        return left.size() > right.size();
    }
    return left > right;
}

std::runtime_error damaged(const fs::path& path, const std::string& problem)
{
    return std::runtime_error("damaged MiniGit repository: '" + path.string()
                              + "' " + problem
                              + ". Fix or remove .minigit before running init again");
}

} // namespace

MiniGitManager::MiniGitManager(const RepositoryPaths& paths,
                               const FileSystem& fileSystem)
    : paths_(paths), fileSystem_(fileSystem)
{
}

void MiniGitManager::init()
{
    const fs::path& metadataRoot = paths_.metadataRoot();

    // Inspect before writing anything. fileStatus does not follow a final link.
    const auto rootStatus = fileSystem_.fileStatus(metadataRoot);
    if (rootStatus.type() == fs::file_type::not_found)
    {
        createRepository();
        std::cout << "Initialized empty MiniGit repository in "
                  << metadataRoot.string() << '\n';
        return;
    }

    if (fs::is_symlink(rootStatus))
    {
        throw damaged(metadataRoot, "is a symbolic link, not a directory");
    }
    if (!fs::is_directory(rootStatus))
    {
        throw damaged(metadataRoot, "exists but is not a directory");
    }

    // Validation only reads; a damaged repository is reported, never repaired.
    validateRepository();
    std::cout << "MiniGit repository already initialized in "
              << metadataRoot.string() << '\n';
}

void MiniGitManager::createRepository() const
{
    const fs::path& metadataRoot = paths_.metadataRoot();

    // create_directories also creates .minigit itself and any missing parents.
    fileSystem_.createDirectories(metadataRoot / "stage");
    fileSystem_.createDirectories(metadataRoot / "commits");

    // HEAD and index.txt start as zero-byte files; next-id.txt ends with LF.
    writeMetadata(paths_.headFile(), "");
    writeMetadata(paths_.indexFile(), "");
    writeMetadata(metadataRoot / "next-id.txt", "1\n");
}

void MiniGitManager::writeMetadata(const std::filesystem::path& path,
                                   const std::string& text) const
{
    const WriteResult result = fileSystem_.writeTextFile(path, text);
    if (!result.ok)
    {
        throw std::runtime_error("could not initialize repository: " + result.error);
    }
}

void MiniGitManager::validateRepository() const
{
    const fs::path& metadataRoot = paths_.metadataRoot();

    requireDirectory(metadataRoot / "stage");
    requireDirectory(metadataRoot / "commits");

    const std::string headText = readMetadata(paths_.headFile());
    const std::string head = withoutFinalLineEnding(headText);
    (void)readMetadata(paths_.indexFile()); // must be a regular, readable file

    const fs::path nextIdFile = metadataRoot / "next-id.txt";
    const std::string nextId = withoutFinalLineEnding(readMetadata(nextIdFile));

    // No commits means a zero-byte HEAD, not an empty line.
    if (!headText.empty() && !isPositiveDecimalId(head))
    {
        throw damaged(paths_.headFile(), "must be empty or hold one positive commit ID");
    }
    if (!isPositiveDecimalId(nextId))
    {
        throw damaged(nextIdFile, "must hold one positive commit ID");
    }
    if (!head.empty() && !isGreaterId(nextId, head))
    {
        throw damaged(nextIdFile, "must be greater than the commit ID in HEAD");
    }
}

void MiniGitManager::requireDirectory(const std::filesystem::path& path) const
{
    const auto status = fileSystem_.fileStatus(path);
    if (status.type() == fs::file_type::not_found)
    {
        throw damaged(path, "is missing");
    }
    if (fs::is_symlink(status))
    {
        throw damaged(path, "is a symbolic link, not a directory");
    }
    if (!fs::is_directory(status))
    {
        throw damaged(path, "is not a directory");
    }
}

std::string MiniGitManager::readMetadata(const std::filesystem::path& path) const
{
    const auto status = fileSystem_.fileStatus(path);
    if (status.type() == fs::file_type::not_found)
    {
        throw damaged(path, "is missing");
    }
    if (fs::is_symlink(status))
    {
        throw damaged(path, "is a symbolic link, not a regular file");
    }
    if (!fs::is_regular_file(status))
    {
        throw damaged(path, "is not a regular file");
    }

    const ReadResult result = fileSystem_.readTextFile(path);
    if (!result.ok)
    {
        throw damaged(path, "could not be read: " + result.error);
    }
    return result.text;
}

} // namespace minigit
