#include "FileSystem.hpp"
#include "TestDirectory.hpp"

#include <filesystem>
#include <iostream>
#include <system_error>
#include <vector>

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

        // FS-6: Test a selected path outside the stage directory.
        auto stagePath = directory.path() / "stage";
        auto indexPath = directory.path() / "index.txt";
        const auto selectedPath = directory.path() / "outside.txt";

        fs::create_directories(stagePath);
        fileSystem.writeTextFile(selectedPath, "content");

        std::vector<std::string> selectedPaths = {selectedPath.string()};
        const bool cleared = fileSystem.clearStage(selectedPaths, stagePath, indexPath);

        if (cleared) {
            std::cerr << "FAILED: clearStage should return false for a path outside the stage directory\n";
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