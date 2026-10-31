#include "PathLab.hpp"
#include <iostream>
#include <string>

bool check(bool condition, const std::string &message)
{
    if (!condition)
    {
        std::cerr << "PathLab check failed: " << message << "\n";
        return false;
    }
    return true;
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

    std::cout << "All PathLab checks passed\n";
    return 0;
}