#pragma once
#include "PathLab.hpp"
#include <istream>

namespace minigit::detail {

// Internal stream-reading step, exposed here so tests can simulate an I/O error.
ReadResult readTextStream(std::istream& input,
                          const std::filesystem::path& sourcePath);

} // namespace minigit::detail
