#include "PathLab.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
}
// Issue 3 test helper: read a whole file back as text.
std::string readTextFile(const std::filesystem::path& filePath) {
    std::ifstream in(filePath, std::ios::binary);
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
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
      // ===== Issue 3: writeTextFile =====
    // Work in a fresh temporary folder so the test never touches the project.
    const std::filesystem::path tempRoot =
        std::filesystem::temp_directory_path() / "minigit-issue3-test";
    std::filesystem::remove_all(tempRoot);

    bool issue3Passed = true;

    // Success case: parent folders a/b/c do not exist yet.
    const std::filesystem::path nestedFile = tempRoot / "a" / "b" / "c" / "note.txt";
    const std::string text = "hello minigit\n";
    const minigit::WriteResult written = minigit::writeTextFile(nestedFile, text);

    issue3Passed = check(written.ok, "writeTextFile should succeed in a new nested directory")
                   && check(std::filesystem::is_directory(nestedFile.parent_path()),
                            "writeTextFile should create the missing parent directories")
                   && check(readTextFile(nestedFile) == text,
                            "the written file should contain exactly the supplied text");

    // Failure case: the "parent" is a regular file, so a directory cannot be created there.
    if (issue3Passed) {
        const std::filesystem::path blocker = tempRoot / "blocker";
        minigit::writeTextFile(blocker, "not a folder");
        const minigit::WriteResult blocked = minigit::writeTextFile(blocker / "child.txt", text);

        issue3Passed = check(!blocked.ok, "writeTextFile should report failure when a parent is a file")
                       && check(!blocked.error.empty(), "a failed write should explain the error");
    }

    // Clean up before returning, whether the checks passed or not.
    std::error_code cleanupError;
    std::filesystem::remove_all(tempRoot, cleanupError);
    if (!issue3Passed) {
        return 1;
    }
    if (!check(!std::filesystem::exists(tempRoot), "temporary test files should be removed")) {
        return 1;
    }

    std::cout << "PathLab smoke check passed\n";
    return 0;
}
