#pragma once

namespace minigit
{

// Parse terminal arguments and translate command failures into exit codes.
class CommandLineApp
{
public:
    // M2 accepts only "minigit init". Errors print "Error: ..." and return 1.
    int run(int argc, char* argv[]) const;
};

} // namespace minigit
