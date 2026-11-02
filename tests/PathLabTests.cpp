#include "PathLab.hpp"
#include <iostream>



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
    const std::filesystem::path root = "course/repo";
    const std::filesystem::path nestedRoot = std::filesystem::path("course") / "projects" / "repo";

    if(!check(minigit::metadataDirectory(root) == root / ".minigit", "metadataDirectory should append .minigit to the root path")){
        return 1;
    }
    

      // ===== Issue 2: headFile =====
    if (!check(minigit::headFile(root) == minigit::metadataDirectory(root) / "HEAD",
               "headFile should append HEAD to the metadata directory")) {
        return 1;
    }

    if (!check(minigit::headFile(nestedRoot) == nestedRoot / ".minigit" / "HEAD",
               "headFile should return .minigit/HEAD under a nested root path")) {
        return 1;
    }

    std::cout << "PathLab smoke check passed\n";
    return 0;
}
