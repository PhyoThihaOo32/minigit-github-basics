#include "CommandLineApp.hpp"
#include "MiniGitManager.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace minigit
{

int CommandLineApp::run(int argc, char* argv[]) const
{
    try
    {
        // Reject unsupported requests before inspecting or changing the project.
        if (argc < 2)
        {
            throw std::runtime_error("No command provided. Usage: minigit init");
        }
        const std::string command = argv[1];
        if (command != "init")
        {
            throw std::runtime_error("Unknown command '" + command
                                     + "'. Usage: minigit init");
        }
        if (argc != 2)
        {
            throw std::runtime_error("init does not accept extra arguments. Usage: minigit init");
        }

        // The manager stores references, so these named dependencies outlive it.
        const RepositoryPaths paths(std::filesystem::current_path());
        const FileSystem fileSystem;
        MiniGitManager manager(paths, fileSystem);
        manager.init();
        return 0;
    }
    catch (const std::exception& failure)
    {
        std::cerr << "Error: " << failure.what() << '\n';
        return 1;
    }
}

} // namespace minigit
