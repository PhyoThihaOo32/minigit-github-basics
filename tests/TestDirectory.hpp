#pragma once
#include <filesystem>
#include <string>
#include <system_error>

// A test owns a directory only after create_directory reports it was created.
// Copying would give two objects cleanup responsibility, so copying is disabled.
class TestDirectory {
public:
    TestDirectory() = default;
    TestDirectory(const TestDirectory&) = delete;
    TestDirectory& operator=(const TestDirectory&) = delete;

    ~TestDirectory() {
        // Fallback cleanup for an early return. Tests also call cleanup directly
        // so a normal cleanup failure becomes a failed check.
        std::error_code ignored;
        cleanup(ignored);
    }

    bool create(const std::filesystem::path& parent,
                const std::string& prefix,
                std::error_code& error) {
        error.clear();
        if (!directory_.empty()) {
            error = std::make_error_code(std::errc::operation_not_permitted);
            return false;
        }
        for (int attempt = 0; attempt < 100; ++attempt) {
            const auto candidate = parent / (prefix + "-" + std::to_string(attempt));
            if (std::filesystem::create_directory(candidate, error)) {
                directory_ = candidate;
                return true;
            }
            // Never delete an existing candidate, even if it is a regular file.
            if (error && error != std::errc::file_exists) {
                return false;
            }
            error.clear();
        }
        error = std::make_error_code(std::errc::file_exists);
        return false;
    }

    const std::filesystem::path& path() const { return directory_; }

    bool cleanup(std::error_code& error) {
        error.clear();
        if (directory_.empty()) {
            return true;
        }
        std::filesystem::remove_all(directory_, error);
        if (error) {
            return false;
        }
        directory_.clear();
        return true;
    }

private:
    std::filesystem::path directory_;
};
