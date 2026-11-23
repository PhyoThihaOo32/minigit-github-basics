#include "CommandLineApp.hpp"

int main(int argc, char* argv[])
{
    const minigit::CommandLineApp application;
    return application.run(argc, argv);
}
