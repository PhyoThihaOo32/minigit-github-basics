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

    std::cout << "All PathLab checks passed\n";
    return 0;
}
