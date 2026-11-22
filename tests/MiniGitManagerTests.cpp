#include "MiniGitManager.hpp"
#include "TestDirectory.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

namespace
{

bool check(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
    }
    return condition;
}

bool checkConstruction(const std::filesystem::path& root)
{
    const minigit::FileSystem fileSystem;
    const auto normalRoot = root / "repo";
    const auto nestedRoot = root / "course" / "team" / "repo";

    for (const auto& projectRoot : {normalRoot, nestedRoot})
    {
        const minigit::RepositoryPaths paths(projectRoot);
        const minigit::MiniGitManager manager(paths, fileSystem);
        (void)manager;
    }

    if (!check(std::filesystem::is_empty(root),
               "constructing managers for missing roots must create nothing"))
    {
        return false;
    }

    const auto existingRoot = root / "existing";
    const minigit::RepositoryPaths paths(existingRoot);
    const auto head = paths.headFile();
    const auto workingFile = existingRoot / "notes.txt";
    if (!check(fileSystem.writeTextFile(head, "7").ok,
               "could not create HEAD fixture")
        || !check(fileSystem.writeTextFile(workingFile, "keep this text\n").ok,
                  "could not create working-file fixture"))
    {
        return false;
    }

    // This deliberately incomplete metadata is for constructor testing only.
    // Validation belongs to init; construction must leave it alone.
    const minigit::MiniGitManager manager(paths, fileSystem);
    (void)manager;

    const auto storedHead = fileSystem.readTextFile(head);
    const auto storedWorkingFile = fileSystem.readTextFile(workingFile);
    return check(storedHead.ok && storedHead.text == "7",
                 "construction must preserve existing HEAD contents")
        && check(storedWorkingFile.ok && storedWorkingFile.text == "keep this text\n",
                 "construction must preserve working files")
        && check(!std::filesystem::exists(paths.indexFile())
                     && !std::filesystem::exists(paths.metadataRoot() / "stage")
                     && !std::filesystem::exists(paths.metadataRoot() / "commits")
                     && !std::filesystem::exists(paths.metadataRoot() / "next-id.txt"),
                 "construction must not initialize or repair repository metadata");
}

} // namespace

int main()
{
    TestDirectory directory;
    std::error_code error;
    if (!directory.create(std::filesystem::temp_directory_path(),
                          "minigit-manager-tests", error))
    {
        std::cerr << "Could not create test directory: " << error.message() << '\n';
        return 1;
    }

    bool passed = false;
    try
    {
        passed = checkConstruction(directory.path());
    }
    catch (const std::exception& failure)
    {
        std::cerr << "Manager construction failed: " << failure.what() << '\n';
    }

    if (!directory.cleanup(error))
    {
        std::cerr << "Could not clean test directory: " << error.message() << '\n';
        return 1;
    }
    if (passed)
    {
        std::cout << "MiniGitManager construction checks passed\n";
    }
    return passed ? 0 : 1;
}
