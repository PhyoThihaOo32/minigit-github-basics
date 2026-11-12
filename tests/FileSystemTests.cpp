#include "FileSystem.hpp"
#include "TestDirectory.hpp"

#include <filesystem>
#include <iostream>
#include <system_error>

int main() {
    namespace fs = std::filesystem;

    try {
        const auto originalDirectory = fs::current_path();
        TestDirectory directory;
        std::error_code error;
        if (!directory.create(fs::temp_directory_path(), "minigit-filesystem-test", error)) {
            std::cerr << "FAILED: could not create test directory: " << error.message() << '\n';
            return 1;
        }

        fs::current_path(directory.path());
        [[maybe_unused]] const minigit::FileSystem fileSystem;
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
        if (!directory.cleanup(error)) {
            std::cerr << "FAILED: could not clean up test directory: " << error.message() << '\n';
            return 1;
        }
    } catch (const fs::filesystem_error& error) {
        std::cerr << "FAILED: " << error.what() << '\n';
        return 1;
    }

    std::cout << "FileSystem construction check passed\n";
    return 0;
}
