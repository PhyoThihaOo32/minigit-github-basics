#include "PathLab.hpp"
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <system_error>

bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
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
    if (!check(minigit::metadataDirectory(root) == root / ".minigit",
        "metadataDirectory should append .minigit to the root path")){
            return 1;
    }
    std::cout << "PathLab smoke check passed\n";

    //testing headPath with normal root:
    if (!check(minigit::headPath(root) == minigit::metadataDirectory(root) / "HEAD",
        "headPath should append .minigit/HEAD to the root path")){
            return 1;
    }
    //testing headPath with nested root:
    const std::filesystem::path nestedRoot = root / "nested";
    if (!check(minigit::headPath(nestedRoot) == minigit::metadataDirectory(nestedRoot) / "HEAD",
        "headPath should append .minigit/HEAD to the nested root path")){
            return 1;
    }

    //issue 3: testing writeToFile with nested directories
     std::error_code error;
    std::filesystem::path tempBase =
        std::filesystem::temp_directory_path(error);

    if (!check(!error, "Temporary directory should be available")) {
        return 1;
    }

    // Give each test run a different starting name
    std::string name = "minigit_test3_" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());

    std::filesystem::path tempFolder, secondFolder;
    bool created = false;
    bool passed = true;

    // Create the first folder without deleting existing folders
    for (int i = 0; i < 100; i++) {
        tempFolder = tempBase / (name + "_" + std::to_string(i));
        error.clear();
        created = std::filesystem::create_directory(tempFolder, error);

        if (created || error) {
            break;
        }
    }

    if (!check(created && !error, "First folder should be created")) {
        return 1;
    }

    // Create a sentinel file to check that collisions are safe
    std::ofstream sentinel(tempFolder / "sentinel.txt");
    sentinel << "Keep this file";
    sentinel.close();

    if (!check(!sentinel.fail(), "Sentinel should be written")) {
        passed = false;
    }

    // Start with the same name to force a collision
    bool secondCreated = false;

    for (int i = 0; i < 100; i++) {
        secondFolder = tempBase / (name + "_" + std::to_string(i));
        error.clear();
        secondCreated = std::filesystem::create_directory(secondFolder, error);

        if (secondCreated || error) {
            break;
        }
    }

    if (!check(secondCreated && !error && secondFolder != tempFolder,
               "Second folder should use a different name")) {
        passed = false;
    }

    if (!check(std::filesystem::exists(tempFolder / "sentinel.txt"),
               "Existing sentinel should remain")) {
        passed = false;
    }

    // Test writeToFile with a nested directory
    const std::filesystem::path filePath =
        tempFolder / "nested" / "notes.txt";

    if (!check(minigit::writeToFile(filePath, "Hello, world!"),
               "writeToFile should write to the file")) {
        passed = false;
    }

    std::ifstream file(filePath);
    std::string content;

    if (!check(file.is_open(), "File should open")) {
        passed = false;
    }
    else {
        std::getline(file, content);
        if (!check(content == "Hello, world!",
                   "File should have the right text")) {
            passed = false;
        }
    }
    file.close();

    // Remove only our first folder
    error.clear();
    std::filesystem::remove_all(tempFolder, error);

    if (!check(!error, "First folder should be removed")) {
        passed = false;
    }

    // The second folder must still exist
    if (secondCreated) {
        if (!check(std::filesystem::exists(secondFolder),
                   "Second folder should still exist")) {
            passed = false;
        }

        error.clear();
        std::filesystem::remove_all(secondFolder, error);

        if (!check(!error, "Second folder should be removed")) {
            passed = false;
        }
    }

    if (!passed) {
        return 1;
    }

    return 0;
}