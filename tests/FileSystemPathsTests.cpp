#include "FileSystem.hpp"
#include "FileStatusResult.hpp"
#include "TestDirectory.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

namespace
{

namespace fs = std::filesystem;

bool check(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
    }
    return condition;
}

template <typename Operation>
bool checkFilesystemError(Operation operation, const std::string& message)
{
    try
    {
        operation();
    }
    catch (const fs::filesystem_error& failure)
    {
        return check(static_cast<bool>(failure.code())
                         && std::string(failure.what()).size() > 0,
                     "filesystem errors must contain a code and explanation");
    }
    catch (const std::exception& failure)
    {
        std::cerr << "FAILED: unexpected exception: " << failure.what() << '\n';
        return false;
    }
    return check(false, message);
}

bool checkPaths(const minigit::FileSystem& fileSystem, const fs::path& root)
{
    const auto missing = root / "missing" / "notes.txt";
    if (!check(fileSystem.fileStatus(missing).type() == fs::file_type::not_found,
               "missing paths should return not_found")
        || !check(fs::is_empty(root), "inspection must create nothing")
        || !check(fs::is_directory(fileSystem.fileStatus(root)),
                  "an existing directory should be reported as a directory"))
    {
        return false;
    }

    const auto nested = root / "course" / "team";
    fileSystem.createDirectories(nested);
    const auto sentinel = nested / "notes.txt";
    if (!check(fs::is_directory(nested), "missing parent directories should be created")
        || !check(fileSystem.writeTextFile(sentinel, "preserve me\n").ok,
                  "could not create sentinel fixture"))
    {
        return false;
    }
    fileSystem.createDirectories(nested);

    const auto sentinelText = fileSystem.readTextFile(sentinel);
    if (!check(fs::is_regular_file(fileSystem.fileStatus(sentinel)),
               "an existing file should be reported as a regular file")
        || !check(sentinelText.ok && sentinelText.text == "preserve me\n",
                  "creating an existing directory must preserve its files")
        || !checkFilesystemError([&] { fileSystem.createDirectories(sentinel); },
                                 "a regular file cannot be used as a directory")
        || !checkFilesystemError([&] { fileSystem.createDirectories(sentinel / "child"); },
                                 "a file blocking a parent directory must report an error"))
    {
        return false;
    }

    const auto preserved = fileSystem.readTextFile(sentinel);
    return check(preserved.ok && preserved.text == "preserve me\n",
                 "failed operations must preserve the existing file");
}

bool checkStatusErrors(const fs::path& root)
{
    const auto path = root / "unreadable";
    const fs::file_status noStatus(fs::file_type::none);
    for (const auto reason : {std::errc::permission_denied, std::errc::io_error,
                              std::errc::not_a_directory})
    {
        const auto error = std::make_error_code(reason);
        try
        {
            minigit::detail::checkedFileStatus(path, noStatus, error);
            return check(false, "inspection errors must reach the caller");
        }
        catch (const fs::filesystem_error& failure)
        {
            if (!check(failure.code() == error && failure.path1() == path,
                       "inspection failures must preserve the error code and path"))
            {
                return false;
            }
        }
    }

    const auto missing = minigit::detail::checkedFileStatus(
        path, noStatus, std::make_error_code(std::errc::no_such_file_or_directory));
    return check(missing.type() == fs::file_type::not_found,
                 "only a missing-path error should become not_found");
}

bool checkLinks(const minigit::FileSystem& fileSystem, const fs::path& root)
{
    const auto target = root / "course" / "team" / "notes.txt";
    const auto fileLink = root / "file-link";
    std::error_code error;
    fs::create_symlink(target, fileLink, error);
    if (error)
    {
#ifdef _WIN32
        // Windows runners may not have permission to create symbolic links.
        std::cout << "Symbolic-link fixtures unavailable: " << error.message() << '\n';
        return true;
#else
        std::cerr << "Could not create symbolic-link fixture: " << error.message() << '\n';
        return false;
#endif
    }

    const auto directoryLink = root / "directory-link";
    const auto danglingLink = root / "dangling-link";
    fs::create_directory_symlink(target.parent_path(), directoryLink);
    fs::create_symlink(root / "absent-target", danglingLink);

    return check(fs::is_symlink(fileSystem.fileStatus(fileLink)),
                 "inspection must report the file link rather than its target")
        && check(fs::is_symlink(fileSystem.fileStatus(directoryLink)),
                 "inspection must report the directory link rather than its target")
        && check(fs::is_symlink(fileSystem.fileStatus(danglingLink)),
                 "a dangling link must still be reported as a symbolic link");
}

} // namespace

int main()
{
    TestDirectory directory;
    std::error_code error;
    if (!directory.create(fs::temp_directory_path(), "minigit-filesystem-paths", error))
    {
        std::cerr << "Could not create test directory: " << error.message() << '\n';
        return 1;
    }

    bool passed = false;
    try
    {
        const minigit::FileSystem fileSystem;
        passed = checkPaths(fileSystem, directory.path())
            && checkStatusErrors(directory.path())
            && checkLinks(fileSystem, directory.path());
    }
    catch (const std::exception& failure)
    {
        std::cerr << "Filesystem path checks failed: " << failure.what() << '\n';
    }

    if (!directory.cleanup(error))
    {
        std::cerr << "Could not clean test directory: " << error.message() << '\n';
        return 1;
    }
    if (passed)
    {
        std::cout << "Filesystem path checks passed\n";
    }
    return passed ? 0 : 1;
}
