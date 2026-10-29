#include "PathLab.hpp"

namespace minigit
{
    std::filesystem::path metadataDirectory(const std::filesystem::path &root)
    {
        return root / ".minigit";
    }
} // namespace minigit