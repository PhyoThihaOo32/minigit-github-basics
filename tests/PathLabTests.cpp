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

    std::cout << "PathLab path checks passed\n";
    return 0;
}
