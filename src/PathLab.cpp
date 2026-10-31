#include "PathLab.hpp"

namespace minigit
{
    std::filesystem::path metadataDirectory(const std::filesystem::path &root)
    {
        return root / ".minigit";
    }

    std::filesystem::path headPath(const std::filesystem::path &root)
    {
        return metadataDirectory(root) / "HEAD";
    }

} // namespace minigit