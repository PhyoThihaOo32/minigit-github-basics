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


bool checkCommitDirectory(const std::filesystem::path& root) {
    const minigit::RepositoryPaths paths(root);
 
    if (paths.commitDirectory("3") != root / ".minigit" / "commits" / "3") {
        std::cerr << "FAILED: commitDirectory for '3'\n";
        return false;
    }
    return true;
}
 
bool checkCommittedFile(const std::filesystem::path& root) {
    const minigit::RepositoryPaths paths(root);
 
    
    if (paths.committedFile("3", "docs/notes.txt")
        != root / ".minigit" / "commits" / "3" / "files" / "docs" / "notes.txt") {
        std::cerr << "FAILED: committedFile for '3', 'docs/notes.txt'\n";
        return false;
    }
 
    if (paths.committedFile("3", "a.txt").parent_path() != paths.commitDirectory("3") / "files") {
        std::cerr << "FAILED: committedFile for '3', 'a.txt' should sit in commitDirectory/files\n";
        return false;
    }
 
    if (std::filesystem::exists(paths.commitDirectory("3"))) {
        std::cerr << "FAILED: commit paths should not create " << paths.commitDirectory("3") << '\n';
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

    std::cout << "RepositoryPaths checks passed\n";
    return 0;
}
