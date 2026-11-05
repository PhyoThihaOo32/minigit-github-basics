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

bool testHeadAndIndexPaths(const std::filesystem::path& root) {
    const minigit::RepositoryPaths paths(root);

    if (paths.headFile() != root / ".minigit" / "HEAD") {
        std::cerr << "FAILED: headFile should be " << root / ".minigit" / "HEAD" << '\n';
        return false;
    }
    if (paths.indexFile() != root / ".minigit" / "index") {
        std::cerr << "FAILED: indexFile should be " << root / ".minigit" / "index" << '\n';
        return false;
    }
    return true;
}


bool checkStageFile(const std::filesystem::path& root) {
    const minigit::RepositoryPaths paths(root);

    if (paths.stageFile("notes.txt") != root / ".minigit" / "stage" / "notes.txt") {
        std::cerr << "FAILED: stageFile for 'notes.txt'\n";
        return false;
    }

    if (paths.stageFile("docs/notes.txt") != root / ".minigit" / "stage" / "docs" / "notes.txt") {
        std::cerr << "FAILED: stageFile for 'docs/notes.txt'\n";
        return false;
    }

    if (paths.stageFile("docs/notes.txt").filename() != "notes.txt") {
        std::cerr << "FAILED: stageFile for filename\n";
        return false;
    }
    return true;
}



int main() {
    const auto normalRoot = std::filesystem::path("course") / "repo";
    const auto nestedRoot = std::filesystem::path("course") / "team" / "repo";

    if (!checkRoots(normalRoot) || !checkRoots(nestedRoot)|| !checkStageFile(normalRoot) || !checkStageFile(nestedRoot)) {
        return 1;
    }
    testHeadAndIndexPaths(normalRoot);
    testHeadAndIndexPaths(nestedRoot);

    std::cout << "RepositoryPaths checks passed\n";
    return 0;
}
