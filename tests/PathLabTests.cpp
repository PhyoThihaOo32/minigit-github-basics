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
    std::cout << "PathLab smoke check passed\n";

    const std::filesystem::path root = "course/repo";
    if (!check(minigit::metadataDirectory(root) == root / ".minigit",
        "metadataDirectory should append .minigit to the root paath")){
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
    return 0;
}
