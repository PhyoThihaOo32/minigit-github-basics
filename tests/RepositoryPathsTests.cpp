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

void testHeadAndIndexPaths(const std::filesystem::path& root) {
    const minigit::RepositoryPaths paths(root);
    const auto expectedHead = root / ".minigit" / "HEAD";
    const auto expectedIndex = root / ".minigit" / "index";
//Verify derived paths for HEAD and index files are correct
    if (paths.headFile() != expectedHead) {
        std::cerr << "FAILED: headFile should return " << expectedHead << '\n';
    }
    if (paths.indexFile() != expectedIndex) {
        std::cerr << "FAILED: indexFile should return " << expectedIndex << '\n';
    }
    //Verify if acceptance creates the correct paths for HEAD and index files
    std::cout << "HEAD file path: " << paths.headFile() << '\n';
    std::cout << "Index file path: " << paths.indexFile() << '\n';
}

int main() {
    const auto normalRoot = std::filesystem::path("course") / "repo";
    const auto nestedRoot = std::filesystem::path("course") / "team" / "repo";

    if (!checkRoots(normalRoot) || !checkRoots(nestedRoot)) {
        return 1;
    }
    
    testHeadAndIndexPaths(normalRoot);
    testHeadAndIndexPaths(nestedRoot);

    std::cout << "RepositoryPaths checks passed\n";
   
        return 0;
}
