#include "PathLab.hpp"
#include <filesystem>
#include <iostream>

bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
}

int main() {
    std::filesystem::path sample = "course/repo";
    if (sample.filename() != "repo") {
        std::cerr << "path smoke check failed\n";
        return 1;
    }

    const std::filesystem::path root = "course/repo";
    const std::filesystem::path nestedRoot = root / "nested";

    if (!check(minigit::metadataDirectory(root) == root / ".minigit",
               "metadataDirectory should append .minigit to the root path")) {
        return 1;
    }

    if (!check(minigit::metadataDirectory(nestedRoot) == nestedRoot / ".minigit",
               "metadataDirectory should append .minigit to a nested root path")) {
        return 1;
    }

    if (!check(minigit::headPath(root) == minigit::metadataDirectory(root) / "HEAD",
               "headPath should append .minigit/HEAD to the root path")) {
        return 1;
    }

    if (!check(minigit::headPath(nestedRoot) == minigit::metadataDirectory(nestedRoot) / "HEAD",
               "headPath should append .minigit/HEAD to the nested root path")) {
        return 1;
    }

    std::cout << "PathLab smoke check passed\n";
    return 0;
}
