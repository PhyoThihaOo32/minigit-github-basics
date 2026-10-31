#include "PathLab.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <iterator>
#include <system_error>

bool check(bool condition, const std::string &message)
{
    if (!condition)
    {
        std::cerr << "PathLab check failed: " << message << "\n";
        return false;
    }
    return true;
}

// Issue 3: verify nested writes, exact contents, error reporting, and cleanup.
bool testWriteText()
{
    std::error_code error;
    const auto tempBase = std::filesystem::temp_directory_path(error);

    if (!check(!error, "temporary directory should be available"))
    {
        return false;
    }

    // Reduce name collisions between test runs in the system temp directory.
    const auto uniqueId =
        std::chrono::steady_clock::now().time_since_epoch().count();

    const auto tempRoot =
        tempBase / ("minigit-write-test-" + std::to_string(uniqueId));

    // Refuse to reuse an existing directory so cleanup only removes test data
    // from a directory this run successfully created.
    const bool created =
        std::filesystem::create_directory(tempRoot, error);

    if (!check(created && !error,
               "a new temporary test directory should be created"))
    {
        return false;
    }

    // The nested parent folders do not exist yet.
    const auto filePath = tempRoot / "nested" / "folder" / "notes.txt";
    const std::string expected = "Hello MiniGit!\nSecond line.\n";

    // Record failures instead of returning early so cleanup still runs.
    bool passed = check(
        minigit::writeText(filePath, expected),
        "writeText should create parent folders and write text");

    // Read independently of writeText to verify the actual on-disk contents.
    {
        std::ifstream input(filePath);

        if (!check(input.is_open(), "the written file should exist"))
        {
            passed = false;
        }
        else
        {
            const std::string actual(
                (std::istreambuf_iterator<char>(input)),
                std::istreambuf_iterator<char>());

            if (!check(actual == expected,
                       "the file contents should match exactly"))
            {
                passed = false;
            }
        }
    } // Close the file before removing it on Windows.

    // A regular file cannot be a parent directory. This failure case works
    // without relying on platform-specific file permissions.
    if (!check(!minigit::writeText(filePath / "blocked.txt", "test"),
               "an invalid parent directory should report failure"))
    {
        passed = false;
    }

    // Clean up even if a check above failed.
    std::error_code cleanupError;
    std::filesystem::remove_all(tempRoot, cleanupError);

    if (!check(!cleanupError, "temporary test files should be removed"))
    {
        passed = false;
    }

    return passed;
}

// Issue 4: read files made by writeText and report failures without crashing.
bool testReadText()
{
    std::error_code error;
    const auto tempBase = std::filesystem::temp_directory_path(error);
    if (!check(!error, "temporary directory should be available for reading tests"))
    {
        return false;
    }

    const auto uniqueId =
        std::chrono::steady_clock::now().time_since_epoch().count();
    const auto tempRoot =
        tempBase / ("minigit-read-test-" + std::to_string(uniqueId));

    // Only remove a directory that this run created.
    const bool created = std::filesystem::create_directory(tempRoot, error);
    if (!check(created && !error,
               "a new temporary reading-test directory should be created"))
    {
        return false;
    }

    // Include blank lines, spaces, and no final newline to catch lost text.
    const auto filePath = tempRoot / "nested" / "notes.txt";
    const std::string expected =
        "Hello MiniGit!\n\n  Second line with spaces.\nLast line";
    bool passed = check(minigit::writeText(filePath, expected),
                        "writeText should create the file used by readText");

    std::string actual = "old contents";
    std::string errorMessage = "old error";
    const bool readSucceeded = minigit::readText(filePath, actual, errorMessage);
    if (!check(readSucceeded && actual == expected && errorMessage.empty(),
               "readText should return exact contents and clear an old error"))
    {
        passed = false;
    }

    const auto missingPath = tempRoot / "missing.txt";
    const bool missingSucceeded =
        minigit::readText(missingPath, actual, errorMessage);
    if (!check(!missingSucceeded && actual.empty() &&
                   errorMessage.find(missingPath.string()) != std::string::npos,
               "a missing file should fail with its path and no stale contents"))
    {
        passed = false;
    }

    // Empty text is a successful read, distinct from a missing-file failure.
    const auto emptyPath = tempRoot / "empty.txt";
    if (!check(minigit::writeText(emptyPath, ""),
               "writeText should create an empty file"))
    {
        passed = false;
    }
    actual = "old contents";
    const bool emptySucceeded = minigit::readText(emptyPath, actual, errorMessage);
    if (!check(emptySucceeded && actual.empty() && errorMessage.empty(),
               "an empty file should succeed and clear the previous error"))
    {
        passed = false;
    }

    // A directory exists but cannot be read as a text file. This avoids
    // relying on operating-system-specific permission settings in the test.
    const bool directorySucceeded =
        minigit::readText(tempRoot, actual, errorMessage);
    if (!check(!directorySucceeded && actual.empty() && !errorMessage.empty(),
               "an unreadable file path should report a useful failure"))
    {
        passed = false;
    }

    // All reader streams have closed before cleanup, including on Windows.
    // Record failures above instead of returning early so cleanup still runs.
    std::error_code cleanupError;
    std::filesystem::remove_all(tempRoot, cleanupError);
    if (!check(!cleanupError, "temporary reading-test files should be removed"))
    {
        passed = false;
    }

    return passed;
}

int main()
{
    // Baseline smoke check.
    std::filesystem::path sample = "course/repo";
    if (sample.filename() != "repo")
    {
        std::cerr << "path smoke check failed\n";
        return 1;
    }

    // Issue 1: metadata directory path.
    const std::filesystem::path root = "course/repo";
    if (!check(minigit::metadataDirectory(root) == root / ".minigit",
               "metadataDirectory should append .minigit to the root path"))
    {
        return 1;
    }

    // Issue 2: HEAD path for a normal root.
    const std::filesystem::path normalRoot = "repo";
    if (!check(minigit::headPath(normalRoot) ==
                   normalRoot / ".minigit" / "HEAD",
               "headPath should return the HEAD path for a normal root"))
    {
        return 1;
    }

    // Issue 2: HEAD path for a nested root.
    const std::filesystem::path nestedRoot =
        std::filesystem::path("course") / "projects" / "repo";

    if (!check(minigit::headPath(nestedRoot) ==
                   nestedRoot / ".minigit" / "HEAD",
               "headPath should return the HEAD path for a nested root"))
    {
        return 1;
    }

    // Return a nonzero exit status so CTest reports a failed writing check.
    if (!testWriteText())
    {
        return 1;
    }

    if (!testReadText())
    {
        return 1;
    }

    std::cout << "All PathLab checks passed\n";
    return 0;
}
