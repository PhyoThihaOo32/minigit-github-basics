#include "MiniGitManager.hpp"
#include "TestDirectory.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <type_traits>
#include <system_error>

namespace
{

using minigit::FileSystem;
using minigit::MiniGitManager;
using minigit::RepositoryPaths;

static_assert(std::is_constructible_v<MiniGitManager,
                                     const RepositoryPaths&, const FileSystem&>,
              "named dependencies must remain usable");
static_assert(!std::is_constructible_v<MiniGitManager, RepositoryPaths, const FileSystem&>
                  && !std::is_constructible_v<MiniGitManager, const RepositoryPaths&, FileSystem>
                  && !std::is_constructible_v<MiniGitManager, RepositoryPaths, FileSystem>,
              "temporary dependencies must be rejected");
static_assert(!std::is_constructible_v<MiniGitManager, const RepositoryPaths, const FileSystem&>
                  && !std::is_constructible_v<MiniGitManager, const RepositoryPaths&, const FileSystem>
                  && !std::is_constructible_v<MiniGitManager, const RepositoryPaths, const FileSystem>,
              "const temporary dependencies must also be rejected");

bool check(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
    }
    return condition;
}

bool checkConstruction(const std::filesystem::path& root)
{
    const minigit::FileSystem fileSystem;
    const auto normalRoot = root / "repo";
    const auto nestedRoot = root / "course" / "team" / "repo";

    for (const auto& projectRoot : {normalRoot, nestedRoot})
    {
        const minigit::RepositoryPaths paths(projectRoot);
        const minigit::MiniGitManager manager(paths, fileSystem);
        (void)manager;
    }

    if (!check(std::filesystem::is_empty(root),
               "constructing managers for missing roots must create nothing"))
    {
        return false;
    }

    const auto existingRoot = root / "existing";
    const minigit::RepositoryPaths paths(existingRoot);
    const auto head = paths.headFile();
    const auto workingFile = existingRoot / "notes.txt";
    if (!check(fileSystem.writeTextFile(head, "7").ok,
               "could not create HEAD fixture")
        || !check(fileSystem.writeTextFile(workingFile, "keep this text\n").ok,
                  "could not create working-file fixture"))
    {
        return false;
    }

    // This deliberately incomplete metadata is for constructor testing only.
    // Validation belongs to init; construction must leave it alone.
    const minigit::MiniGitManager manager(paths, fileSystem);
    (void)manager;

    const auto storedHead = fileSystem.readTextFile(head);
    const auto storedWorkingFile = fileSystem.readTextFile(workingFile);
    return check(storedHead.ok && storedHead.text == "7",
                 "construction must preserve existing HEAD contents")
        && check(storedWorkingFile.ok && storedWorkingFile.text == "keep this text\n",
                 "construction must preserve working files")
        && check(!std::filesystem::exists(paths.indexFile())
                     && !std::filesystem::exists(paths.metadataRoot() / "stage")
                     && !std::filesystem::exists(paths.metadataRoot() / "commits")
                     && !std::filesystem::exists(paths.metadataRoot() / "next-id.txt"),
                 "construction must not initialize or repair repository metadata");
}

//MG2: init()
namespace fs = std::filesystem;

// Redirect std::cout while init() prints, and restore it even if init throws.
class CoutCapture
{
public:
    CoutCapture() : previous_(std::cout.rdbuf(buffer_.rdbuf())) {}
    ~CoutCapture() { std::cout.rdbuf(previous_); }
    CoutCapture(const CoutCapture&) = delete;
    CoutCapture& operator=(const CoutCapture&) = delete;
    std::string text() const { return buffer_.str(); }

private:
    std::ostringstream buffer_;
    std::streambuf* previous_;
};

// Record every entry under root (without following links) and each file's
// bytes, so a test can prove that init changed nothing at all.
std::map<std::string, std::string> snapshot(const fs::path& root)
{
    std::map<std::string, std::string> entries;
    const minigit::FileSystem fileSystem;
    const auto rootStatus = fs::symlink_status(root);
    if (!fs::is_directory(rootStatus))
    {
        const auto file = fileSystem.readTextFile(root);
        entries["."] = file.ok ? "file:" + file.text : "other";
        return entries;
    }
    for (const auto& entry : fs::recursive_directory_iterator(root))
    {
        const auto relative = fs::relative(entry.path(), root).generic_string();
        const auto status = entry.symlink_status();
        if (fs::is_symlink(status))
        {
            entries[relative] = "link";
        }
        else if (fs::is_directory(status))
        {
            entries[relative] = "dir";
        }
        else
        {
            const auto file = fileSystem.readTextFile(entry.path());
            entries[relative] = file.ok ? "file:" + file.text : "unreadable";
        }
    }
    return entries;
}

bool fileHas(const fs::path& path, const std::string& expected)
{
    const auto file = minigit::FileSystem().readTextFile(path);
    return fs::is_regular_file(fs::symlink_status(path)) && file.ok
        && file.text == expected;
}

// Run init and report whether it threw. The exception message must be useful,
// because the CLI prints it after "Error: ".
bool initThrows(minigit::MiniGitManager& manager, std::string& message)
{
    CoutCapture capture;
    try
    {
        manager.init();
    }
    catch (const std::exception& failure)
    {
        message = failure.what();
        return true;
    }
    return false;
}

bool checkFreshInit(const fs::path& root)
{
    const minigit::FileSystem fileSystem;
    const auto projectRoot = root / "fresh";
    if (!check(fs::create_directory(projectRoot), "could not create fresh project"))
    {
        return false;
    }
    const minigit::RepositoryPaths paths(projectRoot);
    minigit::MiniGitManager manager(paths, fileSystem);

    std::string output;
    {
        CoutCapture capture;
        manager.init();
        output = capture.text();
    }

    const auto metadata = projectRoot / ".minigit";
    const std::map<std::string, std::string> expected{
        {"HEAD", "file:"},
        {"commits", "dir"},
        {"index.txt", "file:"},
        {"next-id.txt", "file:1\n"},
        {"stage", "dir"},
    };

    return check(fs::is_directory(fs::symlink_status(metadata)),
                 "init must create .minigit/ in the project root")
        && check(fs::is_directory(fs::symlink_status(metadata / "stage")),
                 "init must create .minigit/stage/")
        && check(fs::is_directory(fs::symlink_status(metadata / "commits")),
                 "init must create .minigit/commits/")
        && check(fileHas(paths.headFile(), ""), "HEAD must exist and be empty")
        && check(fileHas(paths.indexFile(), ""), "index.txt must exist and be empty")
        && check(fileHas(metadata / "next-id.txt", "1\n"),
                 "next-id.txt must contain the first commit ID 1")
        && check(snapshot(metadata) == expected,
                 "init must create exactly the documented layout")
        && check(snapshot(projectRoot).size() == expected.size() + 1,
                 "init must not create anything outside .minigit/")
        && check(output.find("Initialized empty MiniGit repository") != std::string::npos,
                 "a new repository must print a success message");
}

bool checkRepeatedInit(const fs::path& root)
{
    const minigit::FileSystem fileSystem;
    const auto projectRoot = root / "repeated";
    const minigit::RepositoryPaths paths(projectRoot);
    minigit::MiniGitManager manager(paths, fileSystem);

    // First run on an untouched layout: the second run must change nothing.
    {
        CoutCapture capture;
        manager.init();
    }
    const auto untouched = snapshot(projectRoot);
    std::string output;
    {
        CoutCapture capture;
        manager.init();
        output = capture.text();
    }
    if (!check(snapshot(projectRoot) == untouched,
               "a second init must leave a fresh layout unchanged")
        || !check(output.find("already initialized") != std::string::npos,
                  "a repeated init must print an already-initialized message"))
    {
        return false;
    }

    // Simulate later work: selections, staged copies, a saved commit, a working
    // file and an extra file. Repeated init must preserve every byte of it.
    const auto metadata = paths.metadataRoot();
    const bool fixtures =
        fileSystem.writeTextFile(paths.indexFile(), "notes.txt\nsrc/main.cpp\n").ok
        && fileSystem.writeTextFile(paths.stageFile("notes.txt"), "staged notes\n").ok
        && fileSystem.writeTextFile(paths.stageFile("src/main.cpp"), "int main() {}\n").ok
        && fileSystem.writeTextFile(paths.committedFile("1", "notes.txt"), "old notes\n").ok
        && fileSystem.writeTextFile(paths.commitDirectory("1") / "message.txt", "first\n").ok
        && fileSystem.writeTextFile(paths.commitDirectory("1") / "parent.txt", "").ok
        && fileSystem.writeTextFile(paths.headFile(), "1").ok
        && fileSystem.writeTextFile(metadata / "next-id.txt", "2\n").ok
        && fileSystem.writeTextFile(metadata / "extra.txt", "keep me\n").ok
        && fileSystem.writeTextFile(projectRoot / "notes.txt", "working notes\n").ok;
    if (!check(fixtures, "could not create repository fixtures"))
    {
        return false;
    }

    const auto before = snapshot(projectRoot);
    {
        CoutCapture capture;
        manager.init();
    }
    return check(snapshot(projectRoot) == before,
                 "repeated init must not overwrite or delete existing data");
}

bool checkCrlfMetadataIsValid(const fs::path& root)
{
    // Readers accept CRLF and a missing final newline.
    const minigit::FileSystem fileSystem;
    const auto projectRoot = root / "crlf";
    const minigit::RepositoryPaths paths(projectRoot);
    minigit::MiniGitManager manager(paths, fileSystem);
    {
        CoutCapture capture;
        manager.init();
    }
    if (!check(fileSystem.writeTextFile(paths.headFile(), "9\r\n").ok
                   && fileSystem.writeTextFile(paths.metadataRoot() / "next-id.txt", "10").ok,
               "could not create CRLF fixtures"))
    {
        return false;
    }
    const auto before = snapshot(projectRoot);
    std::string message;
    return check(!initThrows(manager, message),
                 "CRLF HEAD and next-id without a final newline must be valid: " + message)
        && check(snapshot(projectRoot) == before, "valid CRLF metadata must be preserved");
}

bool checkMinigitFile(const fs::path& root)
{
    const minigit::FileSystem fileSystem;
    const auto projectRoot = root / "ordinary-file";
    const minigit::RepositoryPaths paths(projectRoot);
    if (!check(fileSystem.writeTextFile(paths.metadataRoot(), "not a repository\n").ok,
               "could not create .minigit file fixture"))
    {
        return false;
    }

    const auto before = snapshot(projectRoot);
    minigit::MiniGitManager manager(paths, fileSystem);
    std::string message;
    return check(initThrows(manager, message),
                 "init must report an error when .minigit is an ordinary file")
        && check(message.find(".minigit") != std::string::npos,
                 "the error must name .minigit")
        && check(snapshot(projectRoot) == before
                     && fileHas(paths.metadataRoot(), "not a repository\n"),
                 "init must not overwrite an ordinary .minigit file");
}

// Each case starts from a valid layout, damages one thing, and expects init to
// throw without repairing or removing anything.
template <typename Damage>
bool checkDamaged(const fs::path& root, const std::string& name, Damage damage)
{
    const minigit::FileSystem fileSystem;
    const auto projectRoot = root / ("damaged-" + name);
    const minigit::RepositoryPaths paths(projectRoot);
    minigit::MiniGitManager manager(paths, fileSystem);
    {
        CoutCapture capture;
        manager.init();
    }
    if (!damage(paths))
    {
        std::cout << "Skipped damaged-repository case (fixture unavailable): " << name << '\n';
        return true;
    }

    const auto before = snapshot(projectRoot);
    std::string message;
    return check(initThrows(manager, message), "init must reject damaged metadata: " + name)
        && check(!message.empty(), "the damaged-repository error must explain the problem")
        && check(snapshot(projectRoot) == before,
                 "init must not repair or remove damaged metadata: " + name);
}

bool checkDamagedRepositories(const fs::path& root)
{
    const minigit::FileSystem fileSystem;
    auto remove = [](const fs::path& path) { return fs::remove(path); };
    auto write = [&](const fs::path& path, const std::string& text) {
        return fileSystem.writeTextFile(path, text).ok;
    };

    bool passed = true;
    passed = checkDamaged(root, "missing-head",
                          [&](const minigit::RepositoryPaths& p) { return remove(p.headFile()); })
        && passed;
    passed = checkDamaged(root, "missing-index",
                          [&](const minigit::RepositoryPaths& p) { return remove(p.indexFile()); })
        && passed;
    passed = checkDamaged(root, "missing-next-id", [&](const minigit::RepositoryPaths& p) {
                 return remove(p.metadataRoot() / "next-id.txt");
             })
        && passed;
    passed = checkDamaged(root, "missing-stage", [&](const minigit::RepositoryPaths& p) {
                 return remove(p.metadataRoot() / "stage");
             })
        && passed;
    passed = checkDamaged(root, "missing-commits", [&](const minigit::RepositoryPaths& p) {
                 return remove(p.metadataRoot() / "commits");
             })
        && passed;
    passed = checkDamaged(root, "stage-is-file", [&](const minigit::RepositoryPaths& p) {
                 const auto stage = p.metadataRoot() / "stage";
                 return remove(stage) && write(stage, "");
             })
        && passed;
    passed = checkDamaged(root, "index-is-directory", [&](const minigit::RepositoryPaths& p) {
                 return remove(p.indexFile()) && fs::create_directory(p.indexFile());
             })
        && passed;
    passed = checkDamaged(root, "head-not-a-number", [&](const minigit::RepositoryPaths& p) {
                 return write(p.headFile(), "abc");
             })
        && passed;
    passed = checkDamaged(root, "head-leading-zero", [&](const minigit::RepositoryPaths& p) {
                 return write(p.headFile(), "01");
             })
        && passed;
    passed = checkDamaged(root, "next-id-zero", [&](const minigit::RepositoryPaths& p) {
                 return write(p.metadataRoot() / "next-id.txt", "0\n");
             })
        && passed;
    passed = checkDamaged(root, "next-id-empty", [&](const minigit::RepositoryPaths& p) {
                 return write(p.metadataRoot() / "next-id.txt", "");
             })
        && passed;
    passed = checkDamaged(root, "next-id-two-lines", [&](const minigit::RepositoryPaths& p) {
                 return write(p.metadataRoot() / "next-id.txt", "1\n2\n");
             })
        && passed;
    passed = checkDamaged(root, "next-id-not-after-head", [&](const minigit::RepositoryPaths& p) {
                 return write(p.headFile(), "5") && write(p.metadataRoot() / "next-id.txt", "5\n");
             })
        && passed;
    passed = checkDamaged(root, "stage-is-link", [&](const minigit::RepositoryPaths& p) {
                 // Windows runners may not allow creating links; skip there.
                 const auto stage = p.metadataRoot() / "stage";
                 const auto target = p.projectRoot() / "elsewhere";
                 std::error_code error;
                 fs::create_directory(target, error);
                 if (error || !remove(stage))
                 {
                     return false;
                 }
                 fs::create_directory_symlink(target, stage, error);
                 if (error)
                 {
                     fs::create_directory(stage); // restore so the case is skipped cleanly
                     return false;
                 }
                 return true;
             })
        && passed;
    return passed;
}


} // namespace

int main()
{
    TestDirectory directory;
    std::error_code error;
    if (!directory.create(std::filesystem::temp_directory_path(),
                          "minigit-manager-tests", error))
    {
        std::cerr << "Could not create test directory: " << error.message() << '\n';
        return 1;
    }

    bool passed = false;
    try
    {
        const auto root = directory.path();
        passed = checkConstruction(directory.path());
        passed = checkConstruction(root);
        passed = checkFreshInit(root) && passed;
        passed = checkRepeatedInit(root) && passed;
        passed = checkCrlfMetadataIsValid(root) && passed;
        passed = checkMinigitFile(root) && passed;
        passed = checkDamagedRepositories(root) && passed;
    }
    catch (const std::exception& failure)
    {
        std::cerr << "Manager construction failed: " << failure.what() << '\n';
    }

    if (!directory.cleanup(error))
    {
        std::cerr << "Could not clean test directory: " << error.message() << '\n';
        return 1;
    }
    if (passed)
    {
        std::cout << "MiniGitManager construction checks passed\n";
    }
    return passed ? 0 : 1;
}
