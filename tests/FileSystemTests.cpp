#include "FileSystem.hpp"
#include "TestDirectory.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

bool writeFixture(const std::filesystem::path& path, const std::string& contents) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << contents;
    return output.good();
}

bool readFixture(const std::filesystem::path& path, std::string& contents) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return false;
    }
    contents.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return !input.bad();
}

bool checkInspectionAndStage(const std::filesystem::path& root) {
    namespace fs = std::filesystem;
    const auto existing = root / "work" / "notes.txt";
    const auto missing = root / "work" / "missing.txt";
    const auto staged = root / ".minigit" / "stage" / "work" / "notes.txt";
    if (!writeFixture(existing, "working copy") || !writeFixture(staged, "staged copy")) {
        std::cerr << "FAILED: could not create inspection fixtures\n";
        return false;
    }

    minigit::FileSystem fileSystem;
    std::vector<fs::path> inspectPaths{existing, missing};
    std::vector<std::string> selectedPaths{"work/notes.txt", "work/missing.txt"};
    std::vector<std::string> results{"stale result"};
    fileSystem.inespectFiles(inspectPaths, selectedPaths, results);
    if (results.size() != 2
        || results[0].find("present: work/notes.txt") != 0
        || results[1].find("missing: work/missing.txt") != 0) {
        std::cerr << "FAILED: inspection should distinguish present and missing selections\n";
        return false;
    }

    std::vector<fs::path> nonFilePaths{root / "work"};
    std::vector<std::string> nonFileSelection{"work"};
    fileSystem.inespectFiles(nonFilePaths, nonFileSelection, results);
    if (results.size() != 1 || results[0].find("could not inspect: work") != 0
        || fileSystem.checkStage(nonFilePaths, nonFileSelection)) {
        std::cerr << "FAILED: directories should be reported as non-file inspection failures\n";
        return false;
    }

    std::vector<fs::path> shortPaths{existing};
    fileSystem.inespectFiles(shortPaths, selectedPaths, results);
    if (results.size() != 2 || results[1].find("missing: work/missing.txt") != 0) {
        std::cerr << "FAILED: a selection without a corresponding path should be missing\n";
        return false;
    }

    std::vector<fs::path> stagedPaths{staged};
    std::vector<std::string> stagedSelection{"work/notes.txt"};
    if (!fileSystem.checkStage(stagedPaths, stagedSelection)) {
        std::cerr << "FAILED: a complete staged selection should pass\n";
        return false;
    }

    std::vector<fs::path> incompleteStage{staged, root / ".minigit" / "stage" / "work" / "missing.txt"};
    if (fileSystem.checkStage(incompleteStage, selectedPaths)) {
        std::cerr << "FAILED: a selection with a missing staged file should fail\n";
        return false;
    }

    std::string contents;
    if (!readFixture(existing, contents) || contents != "working copy"
        || !readFixture(staged, contents) || contents != "staged copy"
        || fs::exists(missing)) {
        std::cerr << "FAILED: inspection changed or created a working/staged file\n";
        return false;
    }
    return true;
}

} // namespace

int main() {
    namespace fs = std::filesystem;

    try {
        const auto originalDirectory = fs::current_path();
        const auto tempRoot = fs::absolute(fs::temp_directory_path());
        TestDirectory directory;
        std::error_code error;
        if (!directory.create(tempRoot, "minigit-filesystem-test", error)) {
            std::cerr << "FAILED: could not create test directory: " << error.message() << '\n';
            return 1;
        }
        fs::current_path(directory.path());
        const minigit::FileSystem fileSystem;
        const bool empty = fs::is_empty(directory.path(), error);

        // Leave the directory before cleanup, including when the check fails.
        fs::current_path(originalDirectory);
        if (error) {
            std::cerr << "FAILED: could not inspect test directory: " << error.message() << '\n';
            return 1;
        }
        if (!empty) {
            std::cerr << "FAILED: FileSystem construction should leave the directory empty\n";
            return 1;
        }
        // FS-2: Test writing and reading a file inside a missing subdirectory.
        const auto filePath = directory.path() / "nested" / "notes.txt";

        const auto written = fileSystem.writeTextFile(filePath, "hello\n");

        if (!written.ok) {
            std::cerr << "FAILED: could not write file\n";
            return 1;
        }

        const auto read = fileSystem.readTextFile(filePath);

        if (!read.ok || read.text != "hello\n") {
            std::cerr << "FAILED: file contents do not match\n";
            return 1;
        }
        // FS-2: Test reading an empty file.
        const auto emptyFile = directory.path() / "empty.txt";

        const auto emptyWritten = fileSystem.writeTextFile(emptyFile, "");

        if (!emptyWritten.ok) {
            std::cerr << "FAILED: could not create empty file\n";
            return 1;
        }

        const auto emptyRead = fileSystem.readTextFile(emptyFile);

        if (!emptyRead.ok || !emptyRead.text.empty()) {
            std::cerr << "FAILED: empty file should be a successful read\n";
            return 1;
        }
        // FS-2: Test reading a nonexistent file.
        const auto missingFile = directory.path() / "missing.txt";

        const auto missingRead = fileSystem.readTextFile(missingFile);

        if (missingRead.ok || !missingRead.text.empty() || missingRead.error.empty()) {
            std::cerr << "FAILED: missing file should return an error\n";
            return 1;
        }
        // FS-3: Test copying a regular file.
        const auto source = directory.path() / "original.txt";
        const auto targetFolder = directory.path() / "backup";
        const auto target = targetFolder / "original.txt";

        fs::create_directories(targetFolder);

        const auto sourceWritten = fileSystem.writeTextFile(source, "hello\n");

        if (!sourceWritten.ok) {
            std::cerr << "FAILED: could not create source file\n";
            return 1;
        }

        fileSystem.copy(source, target);

        const auto sourceRead = fileSystem.readTextFile(source);
        const auto targetRead = fileSystem.readTextFile(target);

        if (!sourceRead.ok || !targetRead.ok ||
            sourceRead.text != "hello\n" ||
            targetRead.text != sourceRead.text) {
            std::cerr << "FAILED: copied file contents do not match\n";
            return 1;
        }

        // FS-3: Test copying a missing source file.
        const auto missingSource = directory.path() / "missingSource.txt";
        const auto missingTarget = targetFolder / "missingCopy.txt";

        bool exceptionThrown = false;

        try {
            fileSystem.copy(missingSource, missingTarget);
        } catch (const fs::filesystem_error&) {
            exceptionThrown = true;
        }

        if (!exceptionThrown || fs::exists(missingTarget)) {
            std::cerr << "FAILED: copying a missing file should throw an exception\n";
            return 1;
        }

        // FS-3: Test copying over an existing target.
        const auto existingTarget = targetFolder / "existing.txt";

        const auto existingWritten = fileSystem.writeTextFile(existingTarget, "original content");

        if (!existingWritten.ok) {
            std::cerr << "FAILED: could not create existing target\n";
            return 1;
        }

        bool overwriteException = false;

        try {
            fileSystem.copy(source, existingTarget);
        } catch (const fs::filesystem_error&) {
            overwriteException = true;
        }

        const auto existingRead = fileSystem.readTextFile(existingTarget);

        if (!overwriteException || !existingRead.ok ||
            existingRead.text != "original content") {
            std::cerr << "FAILED: existing target should not be overwritten\n";
            return 1;
        }

        auto headPath = directory.path() / "HEAD";
        const auto legacyTemp = directory.path() / "HEAD.tmp";
        const auto occupiedScratch = directory.path() / ".minigit-head-update-0";
        const auto sentinel = occupiedScratch / "keep.txt";

        if (!fileSystem.writeTextFile(headPath, "old-id").ok ||
            !fileSystem.writeTextFile(legacyTemp, "existing file").ok ||
            !fileSystem.writeTextFile(sentinel, "owned by another update").ok) {
            std::cerr << "FAILED: could not create HEAD update fixtures\n";
            return 1;
        }

        std::string newID = "new-id";
        const bool headUpdated = fileSystem.updateHEAD(headPath, newID);
        const auto headRead = fileSystem.readTextFile(headPath);
        if (!headUpdated || !headRead.ok || headRead.text != "new-id") {
            std::cerr << "FAILED: updateHEAD should replace the old ID\n";
            return 1;
        }

        const auto preservedTemp = fileSystem.readTextFile(legacyTemp);
        const auto preservedSentinel = fileSystem.readTextFile(sentinel);
        if (!preservedTemp.ok || preservedTemp.text != "existing file" ||
            !preservedSentinel.ok || preservedSentinel.text != "owned by another update" ||
            fs::exists(directory.path() / ".minigit-head-update-1")) {
            std::cerr << "FAILED: updateHEAD should clean up only its own temporary files\n";
            return 1;
        }


       
        auto headDirectory = directory.path() / "headDir";
        fs::create_directories(headDirectory);
        if (fileSystem.updateHEAD(headDirectory, newID) || !fs::is_directory(headDirectory)) {
            std::cerr << "FAILED: updateHEAD should return false for a directory\n";
            return 1;
        }

        std::string emptyID;
        const bool emptyUpdated = fileSystem.updateHEAD(headPath, emptyID);
        const auto headAfterEmpty = fileSystem.readTextFile(headPath);
        if (emptyUpdated || !headAfterEmpty.ok || headAfterEmpty.text != "new-id") {
            std::cerr << "FAILED: updateHEAD should reject an empty ID\n";
            return 1;
        }

        
        auto stagePath = directory.path() / "stage";
        auto indexPath = directory.path() / "index.txt";
        const auto selectedStaged = stagePath / "a.txt";
        const auto unselectedStaged = stagePath / "b.txt";
        const auto workingCopy = directory.path() / "a.txt";
        const std::string indexContents = "a.txt\nb.txt\n";

        if (!fileSystem.writeTextFile(selectedStaged, "staged a").ok ||
            !fileSystem.writeTextFile(unselectedStaged, "staged b").ok ||
            !fileSystem.writeTextFile(workingCopy, "working a").ok ||
            !fileSystem.writeTextFile(indexPath, indexContents).ok) {
            std::cerr << "FAILED: could not create stage files\n";
            return 1;
        }

        std::vector<std::string> toClear = {"a.txt"};
        if (!fileSystem.clearStage(toClear, stagePath, indexPath)) {
            std::cerr << "FAILED: clearStage should succeed for a staged file\n";
            return 1;
        }

        const auto unselectedRead = fileSystem.readTextFile(unselectedStaged);
        const auto workingRead = fileSystem.readTextFile(workingCopy);
        const auto indexRead = fileSystem.readTextFile(indexPath);

        if (fs::exists(selectedStaged) ||
            !unselectedRead.ok || unselectedRead.text != "staged b" ||
            !workingRead.ok || workingRead.text != "working a" ||
            !indexRead.ok || indexRead.text != indexContents) {
            std::cerr << "FAILED: clearStage should remove only the selected staged file\n";
            return 1;
        }
        const auto duplicateStaged = stagePath / "d.txt";
        if (!fileSystem.writeTextFile(duplicateStaged, "staged d").ok) {
            std::cerr << "FAILED: could not create duplicate staged file\n";
            return 1;
        }

        std::vector<std::string> duplicateSelection = {"d.txt", "d.txt"};
        if (!fileSystem.clearStage(duplicateSelection, stagePath, indexPath) ||
            fs::exists(duplicateStaged)) {
            std::cerr << "FAILED: clearStage should handle a duplicated selection\n";
            return 1;
        }


        const auto outsidePath = directory.path() / "outside.txt";
        if (!fileSystem.writeTextFile(outsidePath, "content").ok) {
            std::cerr << "FAILED: could not create outside file\n";
            return 1;
        }

        std::vector<std::string> outsideSelection = {(fs::path("..") / "outside.txt").string()};
        std::vector<std::string> absoluteSelection = {outsidePath.string()};
        if (fileSystem.clearStage(outsideSelection, stagePath, indexPath) ||
            fileSystem.clearStage(absoluteSelection, stagePath, indexPath) ||
            !fs::exists(outsidePath)) {
            std::cerr << "FAILED: clearStage should return false for a path outside the stage directory\n";
            return 1;
        }


        const auto nestedStaged = stagePath / "dir" / "c.txt";
        if (!fileSystem.writeTextFile(nestedStaged, "staged c").ok) {
            std::cerr << "FAILED: could not create nested staged file\n";
            return 1;
        }

        std::vector<std::string> directorySelection = {"dir"};
        if (fileSystem.clearStage(directorySelection, stagePath, indexPath) ||
            !fs::exists(nestedStaged)) {
            std::cerr << "FAILED: clearStage should reject a directory selection\n";
            return 1;
        }

        auto innerIndex = stagePath / "index.txt";
        if (!fileSystem.writeTextFile(innerIndex, indexContents).ok) {
            std::cerr << "FAILED: could not create inner index\n";
            return 1;
        }

        std::vector<std::string> indexSelection = {"index.txt"};
        if (fileSystem.clearStage(indexSelection, stagePath, innerIndex) ||
            !fs::exists(innerIndex)) {
            std::cerr << "FAILED: clearStage must refuse to delete the index\n";
            return 1;
        }

        if (!checkInspectionAndStage(directory.path() / "inspection")) {
            return 1;
        }

        if (!directory.cleanup(error)) {
            std::cerr << "FAILED: could not clean up test directory: " << error.message() << '\n';
            return 1;
        }

    } catch (const fs::filesystem_error& error) {
        std::cerr << "FAILED: " << error.what() << '\n';
        return 1;
    }

    std::cout << "All FileSystem checks passed\n";
    return 0;
}
