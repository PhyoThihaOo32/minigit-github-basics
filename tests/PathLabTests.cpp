#include "PathLab.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <chrono>
#include <system_error>

bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
}

// Atomically claim a new directory. An existing candidate belongs to someone
// else, so try another name instead of deleting or reusing it.
bool createTestDirectory(const std::filesystem::path& parent,
                         const std::string& prefix,
                         std::filesystem::path& ownedDirectory,
                         std::error_code& error) {
    ownedDirectory.clear();
    error.clear();
    for (int attempt = 0; attempt < 100; ++attempt) {
        const auto candidate = parent / (prefix + "-" + std::to_string(attempt));
        if (std::filesystem::create_directory(candidate, error)) {
            ownedDirectory = candidate;
            return true;
        }
        if (error && error != std::errc::file_exists) {
            return false;
        }
        error.clear();
    }
    error = std::make_error_code(std::errc::file_exists);
    return false;
}

// This function does not own tempFolder. Returning on failure is safe because
// main cleans it up after this function (and its file stream) have returned.
bool testWriteToFile(const std::filesystem::path& tempFolder) {
    const auto filePath = tempFolder / "nested" / "notes.txt";
    if (!check(minigit::writeToFile(filePath, "Hello, world!"),
               "writeToFile should write to the file")) {
        return false;
    }

    std::ifstream file(filePath);
    if (!check(file.is_open(), "file should open")) {
        return false;
    }
    std::string content;
    std::getline(file, content);
    return check(content == "Hello, world!", "The file should have the right text content");
}

// Force a collision and keep two claimed directories alive at the same time.
// All fixtures live inside this run's owned directory, not the shared temp root.
bool testDirectoryIsolation(const std::filesystem::path& tempFolder) {
    const auto existingDirectory = tempFolder / "collision-0";
    const auto sentinelPath = existingDirectory / "keep.txt";
    if (!check(minigit::writeToFile(sentinelPath, "keep me"),
               "the existing-directory fixture should be created")) {
        return false;
    }

    std::error_code error;
    std::filesystem::path firstDirectory;
    std::filesystem::path secondDirectory;
    if (!check(createTestDirectory(tempFolder, "collision", firstDirectory, error),
               "directory creation should skip an existing candidate")) {
        return false;
    }
    if (!check(createTestDirectory(tempFolder, "collision", secondDirectory, error),
               "a second directory should be created while the first exists")) {
        return false;
    }
    if (!check(firstDirectory != existingDirectory &&
               secondDirectory != existingDirectory &&
               firstDirectory != secondDirectory,
               "each claimed directory should have a separate path")) {
        return false;
    }

    std::filesystem::remove_all(firstDirectory, error);
    if (!check(!error, "the first claimed directory should be removed")) {
        return false;
    }
    const bool secondStillExists = std::filesystem::exists(secondDirectory, error);
    if (!check(secondStillExists && !error,
               "cleaning one directory should leave the other intact")) {
        return false;
    }

    std::ifstream sentinel(sentinelPath);
    std::string contents;
    std::getline(sentinel, contents);
    return check(!sentinel.fail() && contents == "keep me",
                 "an existing candidate and its contents should survive a collision");
}

int main() {
    // Baseline smoke check: the project and CTest wiring work before students
    // begin the path Issues. Add focused assertions with each Issue.
    std::filesystem::path sample = "course/repo";
    if (sample.filename() != "repo") {
        std::cerr << "path smoke check failed\n";
        return 1;
    }
    std::cout << "PathLab smoke check passed\n";

    const std::filesystem::path root = "course/repo";
    const std::filesystem::path nestedRoot = root / "nested";
    if (!check(minigit::metadataDirectory(root) == root / ".minigit",
        "metadataDirectory should append .minigit to the root path")){
            return 1;
    }
    if (!check(minigit::metadataDirectory(nestedRoot) == nestedRoot / ".minigit",
        "metadataDirectory should append .minigit to a nested root path")){
            return 1;
    }
    std::cout << "PathLab smoke check passed\n";

    //testing headPath with normal root:
    if (!check(minigit::headPath(root) == minigit::metadataDirectory(root) / "HEAD",
        "headPath should append .minigit/HEAD to the root path")){
            return 1;
    }
    //testing headPath with nested root:
    if (!check(minigit::headPath(nestedRoot) == minigit::metadataDirectory(nestedRoot) / "HEAD",
        "headPath should append .minigit/HEAD to the nested root path")){
            return 1;
    }

    // Issue 3: create a directory owned by this run before writing any fixtures.
    std::error_code error;
    const auto tempBase = std::filesystem::temp_directory_path(error);
    if (!check(!error, "the system temporary directory should be available")) {
        return 1;
    }
    const auto runId = std::chrono::steady_clock::now().time_since_epoch().count();
    std::filesystem::path tempFolder;
    if (!check(createTestDirectory(tempBase, "minigit_test3-" + std::to_string(runId),
                                   tempFolder, error),
               "this run should create its own temporary directory")) {
        return 1;
    }

    bool passed = testWriteToFile(tempFolder);
    if (!testDirectoryIsolation(tempFolder)) {
        passed = false;
    }

    // Every check above returns here, even on failure. File streams have already
    // closed, and only the directory successfully claimed by this run is removed.
    std::filesystem::remove_all(tempFolder, error);
    if (!check(!error, "this run's temporary directory should be cleaned up")) {
        passed = false;
    }
    return passed ? 0 : 1;
}
