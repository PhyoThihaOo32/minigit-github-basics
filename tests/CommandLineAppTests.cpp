#include "CommandLineApp.hpp"
#include "FileSystem.hpp"
#include "TestDirectory.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

namespace
{

namespace fs = std::filesystem;

bool check(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
    }
    return condition;
}

// Restore the working folder before the owned test directory is cleaned up.
class WorkingDirectory
{
public:
    explicit WorkingDirectory(const fs::path& path) : previous_(fs::current_path())
    {
        fs::current_path(path);
    }
    ~WorkingDirectory()
    {
        std::error_code ignored;
        fs::current_path(previous_, ignored);
    }
    WorkingDirectory(const WorkingDirectory&) = delete;
    WorkingDirectory& operator=(const WorkingDirectory&) = delete;

private:
    fs::path previous_;
};

class OutputCapture
{
public:
    OutputCapture()
        : previousOut_(std::cout.rdbuf(output_.rdbuf())),
          previousError_(std::cerr.rdbuf(error_.rdbuf())) {}
    ~OutputCapture()
    {
        std::cout.rdbuf(previousOut_);
        std::cerr.rdbuf(previousError_);
    }
    OutputCapture(const OutputCapture&) = delete;
    OutputCapture& operator=(const OutputCapture&) = delete;
    std::string output() const { return output_.str(); }
    std::string error() const { return error_.str(); }

private:
    std::ostringstream output_;
    std::ostringstream error_;
    std::streambuf* previousOut_;
    std::streambuf* previousError_;
};

struct RunResult
{
    int exitCode;
    std::string output;
    std::string error;
};

RunResult run(std::vector<std::string> arguments)
{
    // Match main's writable argv strings, including the final null pointer.
    std::vector<char*> argv;
    for (auto& argument : arguments)
    {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);

    const minigit::CommandLineApp app;
    OutputCapture capture;
    const int exitCode = app.run(static_cast<int>(arguments.size()), argv.data());
    return {exitCode, capture.output(), capture.error()};
}

bool checkRejectedArguments()
{
    const std::vector<std::vector<std::string>> cases{
        {"minigit"},
        {"minigit", "status"},
        {"minigit", "unknown"},
        {"minigit", "init", "extra"},
        {"minigit", "add"},
        {"minigit", "commit"},
        {"minigit", "log"},
    };

    bool passed = true;
    for (const auto& arguments : cases)
    {
        const auto result = run(arguments);
        passed = check(result.exitCode == 1, "invalid arguments must return 1") && passed;
        passed = check(result.error.rfind("Error: ", 0) == 0,
                       "invalid arguments must explain the error on stderr") && passed;
        passed = check(result.output.empty(), "invalid arguments must not print success") && passed;
        passed = check(fs::is_empty(fs::current_path()),
                       "invalid arguments must not create metadata or other files") && passed;
    }
    return passed;
}

bool checkInitDispatch()
{
    const auto first = run({"minigit", "init"});
    bool passed = check(first.exitCode == 0 && first.error.empty(),
                        "a new init must succeed without stderr");
    passed = check(first.output.find("Initialized empty MiniGit repository") != std::string::npos,
                   "init must reach the manager and print its success message") && passed;
    passed = check(fs::is_regular_file(fs::path(".minigit") / "HEAD"),
                   "init must create metadata in the current working folder") && passed;

    const minigit::FileSystem fileSystem;
    const auto sentinel = fs::path(".minigit") / "keep.txt";
    if (!check(fileSystem.writeTextFile(sentinel, "keep these bytes\r\n").ok,
               "could not create repeat-init fixture"))
    {
        return false;
    }
    const auto second = run({"minigit", "init"});
    const auto stored = fileSystem.readTextFile(sentinel);
    passed = check(second.exitCode == 0 && second.error.empty(),
                   "a repeated valid init must succeed without stderr") && passed;
    passed = check(second.output.find("already initialized") != std::string::npos,
                   "a repeated init must print the manager's message") && passed;
    passed = check(stored.ok && stored.text == "keep these bytes\r\n",
                   "a repeated init must preserve existing bytes") && passed;

    if (!check(fs::remove(fs::path(".minigit") / "HEAD"),
               "could not create damaged-repository fixture"))
    {
        return false;
    }
    const auto damaged = run({"minigit", "init"});
    passed = check(damaged.exitCode == 1 && damaged.output.empty(),
                   "an init exception must become exit 1 without success output") && passed;
    passed = check(damaged.error.rfind("Error: ", 0) == 0
                       && damaged.error.find("HEAD") != std::string::npos,
                   "the CLI must keep the manager's useful failure explanation") && passed;
    passed = check(!fs::exists(fs::path(".minigit") / "HEAD"),
                   "the CLI must not repair damaged metadata") && passed;

    const auto unknown = run({"minigit", "status"});
    passed = check(unknown.exitCode == 1 && unknown.error.find("Unknown command 'status'")
                       != std::string::npos,
                   "argument errors must be reported before repository inspection") && passed;
    return passed;
}

} // namespace

int main()
{
    TestDirectory directory;
    std::error_code error;
    if (!directory.create(fs::temp_directory_path(), "minigit-cli-tests", error))
    {
        std::cerr << "Could not create test directory: " << error.message() << '\n';
        return 1;
    }

    bool passed = false;
    try
    {
        WorkingDirectory workingDirectory(directory.path());
        passed = checkRejectedArguments();
        passed = checkInitDispatch() && passed;
    }
    catch (const std::exception& failure)
    {
        passed = false;
        std::cerr << "CommandLineApp checks failed: " << failure.what() << '\n';
    }

    if (!directory.cleanup(error))
    {
        std::cerr << "Could not clean test directory: " << error.message() << '\n';
        return 1;
    }
    if (passed)
    {
        std::cout << "CommandLineApp checks passed\n";
    }
    return passed ? 0 : 1;
}
