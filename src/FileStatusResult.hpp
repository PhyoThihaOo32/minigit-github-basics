#pragma once

#include <filesystem>
#include <system_error>

namespace minigit::detail
{

// Keep error handling testable without platform-specific permission fixtures.
std::filesystem::file_status checkedFileStatus(
    const std::filesystem::path& path,
    std::filesystem::file_status status,
    std::error_code error);

} // namespace minigit::detail
