#include "FileSystem.hpp"
#include "RepositoryPaths.hpp"
#include "TestDirectory.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {

bool checkEntries(const minigit::FileSystem& fileSystem,
                  std::filesystem::path indexPath,
                  const std::string& contents,
                  const std::vector<std::string>& expected) {
    std::ofstream output(indexPath, std::ios::binary);
    output << contents;
    output.close();
    if (!output) {
        std::cerr << "FAILED: could not write index test fixture\n";
        return false;
    }

    const auto originalPath = indexPath;
    if (fileSystem.readIndex(indexPath) != expected || indexPath != originalPath) {
        std::cerr << "FAILED: index entries differ or the supplied path changed\n";
        return false;
    }

    const auto after = fileSystem.readTextFile(indexPath);
    if (!after.ok || after.text != contents) {
        std::cerr << "FAILED: reading the index changed its contents\n";
        return false;
    }
    return true;
}

bool reportsReadError(const minigit::FileSystem& fileSystem, std::filesystem::path path) {
    try {
        fileSystem.readIndex(path);
    } catch (const std::runtime_error& error) {
        if (std::string(error.what()).find(path.filename().string()) != std::string::npos) {
            return true;
        }
    }
    std::cerr << "FAILED: expected a read error identifying " << path << '\n';
    return false;
}

} // namespace

int main() {
    namespace fs = std::filesystem;

    try {
        TestDirectory directory;
        std::error_code error;
        if (!directory.create(fs::temp_directory_path(), "minigit-read-index-test", error)) {
            std::cerr << "FAILED: could not create test directory: " << error.message() << '\n';
            return 1;
        }

        const minigit::FileSystem fileSystem;
        const minigit::RepositoryPaths repositoryPaths(directory.path());
        fs::create_directory(repositoryPaths.metadataRoot());
        const auto indexPath = repositoryPaths.indexFile();

        if (!reportsReadError(fileSystem, indexPath) || fs::exists(indexPath)) {
            std::cerr << "FAILED: a missing index must report an error and remain missing\n";
            return 1;
        }
        if (!checkEntries(fileSystem, indexPath, "src/main.cpp\nREADME.md\n",
                          {"src/main.cpp", "README.md"})
            || !checkEntries(fileSystem, indexPath, "", {})
            || !checkEntries(fileSystem, indexPath, "docs/team notes.txt\r\nREADME.md",
                             {"docs/team notes.txt", "README.md"})) {
            return 1;
        }
        if (!reportsReadError(fileSystem, repositoryPaths.metadataRoot())) {
            return 1;
        }
        if (!directory.cleanup(error)) {
            std::cerr << "FAILED: could not clean up test directory: " << error.message() << '\n';
            return 1;
        }
    } catch (const std::exception& error) {
        std::cerr << "FAILED: " << error.what() << '\n';
        return 1;
    }

    std::cout << "Index read checks passed\n";
    return 0;
}
