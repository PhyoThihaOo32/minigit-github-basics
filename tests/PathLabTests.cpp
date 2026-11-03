#include "PathLab.hpp"
#include "TestDirectory.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <system_error>

bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
}

bool testPaths() {
    const std::filesystem::path root = "repo";
    const auto nested = std::filesystem::path("course") / "projects" / "repo";
    return check(minigit::metadataDirectory(root) == root / ".minigit",
                 "metadataDirectory should append .minigit")
        && check(minigit::headPath(root) == root / ".minigit" / "HEAD",
                 "main's headPath API should work for a normal root")
        && check(minigit::headPath(nested) == nested / ".minigit" / "HEAD",
                 "main's headPath API should work for a nested root")
        && check(minigit::headFile(root) == minigit::headPath(root) &&
                 minigit::headFile(nested) == minigit::headPath(nested),
                 "headFile should stay compatible with headPath");
}

bool testWriting(const std::filesystem::path& root) {
    const auto nestedFile = root / "a" / "b" / "c" / "note.txt";
    const std::string expected = "hello minigit\n";
    const auto written = minigit::writeTextFile(nestedFile, expected);
    if (!check(written.ok && written.error.empty(), "nested writing should succeed")) {
        return false;
    }
    std::ifstream input(nestedFile, std::ios::binary);
    if (!check(input.is_open(), "the written file should open independently")) {
        return false;
    }
    const std::string actual((std::istreambuf_iterator<char>(input)),
                             std::istreambuf_iterator<char>());
    if (!check(actual == expected, "the complete written text should match")) {
        return false;
    }

    const auto blocker = root / "blocker";
    if (!check(minigit::writeTextFile(blocker, "not a folder").ok,
               "the invalid-parent fixture should be created")) {
        return false;
    }
    const auto blocked = minigit::writeTextFile(blocker / "child.txt", expected);
    return check(!blocked.ok && !blocked.error.empty(),
                 "writing below a regular file should return an error")
        && check(!minigit::writeToFile(blocker / "child.txt", expected),
                 "the existing bool writer should also report a failed write");
}

bool testReading(const std::filesystem::path& root) {
    const auto file = root / "notes" / "hello.txt";
    const std::string expected = " first line\n\nsecond line without a final newline";
    if (!check(minigit::writeTextFile(file, expected).ok,
               "the writer should create the file used by the reader")) {
        return false;
    }
    const auto read = minigit::readTextFile(file);
    if (!check(read.ok && read.text == expected && read.error.empty(),
               "reading should preserve spaces, blank lines, and the final character")) {
        return false;
    }

    // Files written through main's bool API must use the same text convention
    // as the structured writer, including on Windows.
    const auto compatibilityPath = root / "existing-writer.txt";
    if (!check(minigit::writeToFile(compatibilityPath, expected),
               "the existing bool writer should create a readable text file")) {
        return false;
    }
    const auto compatibilityRead = minigit::readTextFile(compatibilityPath);
    if (!check(compatibilityRead.ok && compatibilityRead.text == expected,
               "the reader should preserve exact text from the existing writer")) {
        return false;
    }

    const auto missing = minigit::readTextFile(root / "missing.txt");
    if (!check(!missing.ok && missing.text.empty() && !missing.error.empty(),
               "a missing file should return an error without partial text")) {
        return false;
    }
    const auto folder = minigit::readTextFile(file.parent_path());
    if (!check(!folder.ok && folder.text.empty() && !folder.error.empty(),
               "a directory should not be read as a text file")) {
        return false;
    }

    const auto emptyPath = root / "empty.txt";
    if (!check(minigit::writeTextFile(emptyPath, "").ok, "an empty file should be created")) {
        return false;
    }
    const auto empty = minigit::readTextFile(emptyPath);
    return check(empty.ok && empty.text.empty() && empty.error.empty(),
                 "an empty file should be a successful read");
}

// Returning false simulates a failed check. The directory object must still
// remove its own files before control returns to the caller.
bool simulateFailedCheck(const std::filesystem::path& parent,
                         std::filesystem::path& createdPath) {
    TestDirectory directory;
    std::error_code error;
    if (!directory.create(parent, "failed-check", error)) {
        return false;
    }
    createdPath = directory.path();
    minigit::writeTextFile(createdPath / "fixture.txt", "test data");
    return false;
}

bool testDirectoryOwnership(const std::filesystem::path& root) {
    const auto existing = root / "candidate-0";
    const auto sentinel = existing / "keep.txt";
    if (!check(minigit::writeTextFile(sentinel, "keep me").ok,
               "an existing candidate fixture should be created")) {
        return false;
    }

    TestDirectory first;
    TestDirectory second;
    std::error_code error;
    if (!check(first.create(root, "candidate", error) &&
               second.create(root, "candidate", error),
               "two runs should claim separate directories despite a collision")) {
        return false;
    }
    if (!check(first.path() != existing && second.path() != existing &&
               first.path() != second.path(), "existing candidates must never be reused")) {
        return false;
    }
    if (!check(first.cleanup(error), "the first owned directory should be removed")) {
        return false;
    }
    const bool secondExists = std::filesystem::exists(second.path(), error);
    const auto preserved = minigit::readTextFile(sentinel);
    if (!check(secondExists && !error && preserved.ok && preserved.text == "keep me",
               "cleanup should preserve another run and an existing sentinel")) {
        return false;
    }
    if (!check(second.cleanup(error), "the second owned directory should be removed")) {
        return false;
    }

    std::filesystem::path failedPath;
    const bool failedCheck = simulateFailedCheck(root, failedPath);
    const bool leftover = std::filesystem::exists(failedPath, error);
    return check(!failedCheck && !failedPath.empty() && !leftover && !error,
                 "a failed check should still clean its owned directory");
}

int main() {
    if (!testPaths()) {
        return 1;
    }
    std::error_code error;
    const auto tempBase = std::filesystem::temp_directory_path(error);
    if (!check(!error, "the system temporary directory should be available")) {
        return 1;
    }
    const auto runId = std::chrono::steady_clock::now().time_since_epoch().count();
    TestDirectory temporary;
    if (!check(temporary.create(tempBase, "minigit-project-test-" + std::to_string(runId), error),
               "this run should claim its own temporary directory")) {
        return 1;
    }

    bool passed = testWriting(temporary.path() / "issue3");
    if (!testReading(temporary.path() / "issue4")) {
        passed = false;
    }
    if (!testDirectoryOwnership(temporary.path())) {
        passed = false;
    }
    if (!check(temporary.cleanup(error), "all owned test files should be removed")) {
        passed = false;
    }
    if (passed) {
        std::cout << "All PathLab checks passed\n";
    }
    return passed ? 0 : 1;
}
