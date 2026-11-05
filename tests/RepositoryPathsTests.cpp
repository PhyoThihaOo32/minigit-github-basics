#include "RepositoryPaths.hpp"
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

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

bool checkAccepted(const minigit::RepositoryPaths& paths,
                   const std::string& argument,
                   const std::string& expected) {
    try {
        const auto actual = paths.safeRelativeFile(argument);
        if (actual != expected) {
            std::cerr << "FAILED: \"" << argument << "\" returned \"" << actual
                      << "\", expected \"" << expected << "\"\n";
            return false;
        }
    } catch (const std::exception& error) {
        std::cerr << "FAILED: accepted path \"" << argument
                  << "\" threw: " << error.what() << '\n';
        return false;
    }
    return true;
}

bool checkRejected(const minigit::RepositoryPaths& paths,
                   const std::string& argument) {
    try {
        paths.safeRelativeFile(argument);
    } catch (const std::runtime_error& error) {
        if (std::string(error.what()).empty()) {
            std::cerr << "FAILED: rejection needs an error message\n";
            return false;
        }
        return true;
    } catch (const std::exception& error) {
        std::cerr << "FAILED: wrong exception type for \"" << argument
                  << "\": " << error.what() << '\n';
        return false;
    }
    std::cerr << "FAILED: unsafe path \"" << argument << "\" was accepted\n";
    return false;
}

bool checkSafeRelativeFiles(const std::filesystem::path& root) {
    const minigit::RepositoryPaths paths(root);
    bool passed = true;

    struct AcceptedCase {
        const char* argument;
        const char* expected;
    };
    const AcceptedCase accepted[] = {
        {"notes.txt", "notes.txt"},
        {"docs/notes.txt", "docs/notes.txt"},
        {"docs/./notes.txt", "docs/notes.txt"},
        {"docs/../notes.txt", "notes.txt"},
        {"my..notes.txt", "my..notes.txt"}
    };
    for (const auto& test : accepted) {
        if (!checkAccepted(paths, test.argument, test.expected)) {
            passed = false;
        }
    }

    const char* rejected[] = {
        "",
        ".",
        "/notes.txt",
        "../notes.txt",
        "docs/../../notes.txt",
        ".minigit/HEAD",
        ".git/config",
        "docs/.minigit/x",
        "docs/..",
        ".git/../notes.txt",
        ".minigit/../notes.txt"
    };
    for (const auto* argument : rejected) {
        if (!checkRejected(paths, argument)) {
            passed = false;
        }
    }

#ifdef _WIN32
    // Windows supports drive roots and backslash separators as well.
    if (!checkAccepted(paths, "docs\\notes.txt", "docs/notes.txt")) {
        passed = false;
    }
    const char* windowsRejected[] = {"C:\\notes.txt", "C:notes.txt", "\\notes.txt"};
    for (const auto* argument : windowsRejected) {
        if (!checkRejected(paths, argument)) {
            passed = false;
        }
    }
#endif

    return passed;
}

int main() {
    const auto normalRoot = std::filesystem::path("course") / "repo";
    const auto nestedRoot = std::filesystem::path("course") / "team" / "repo";

    if (!checkRoots(normalRoot) || !checkRoots(nestedRoot)
        || !checkStageFile(normalRoot) || !checkStageFile(nestedRoot)) {
        return 1;
    }

    const bool passed = checkSafeRelativeFiles(normalRoot)
        && checkSafeRelativeFiles(nestedRoot);
    if (passed) {
        std::cout << "RepositoryPaths checks passed\n";
    }
    return passed ? 0 : 1;
}
