#include "RepositoryPaths.hpp"
#include <iostream>

bool checkRoots(const std::filesystem::path& root) {
    const minigit::RepositoryPaths paths(root);

    if (paths.projectRoot() != root) {
        std::cerr << "FAILED: projectRoot should preserve " << root << '\n';
        return false;
    }
    if (paths.metadataRoot() != root / ".minigit") {
        std::cerr << "FAILED: metadataRoot should append .minigit to " << root << '\n';
        return false;
    }
    return true;
}

int main() {
    const auto normalRoot = std::filesystem::path("course") / "repo";
    const auto nestedRoot = std::filesystem::path("course") / "team" / "repo";

    if (!checkRoots(normalRoot) || !checkRoots(nestedRoot)) {
        return 1;
    }

    std::cout << "RepositoryPaths checks passed\n";
    return 0;
}
