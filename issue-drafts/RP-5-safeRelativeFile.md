# Issue RP-5 Add safeRelativeFile(argument): text rules

## Goal

Check a path typed by the user (for example after `minigit add`) and return a clean relative path, or throw if it is unsafe.

## Background

A user could type `../secret.txt` or `.minigit/HEAD` and make MiniGit touch files outside the project or inside its own data. This issue checks the path text only; it does not touch the disk.

## Dependency

RP-1 merged.

## Functions to add

- `std::string safeRelativeFile(const std::string& argument) const;`

Throw `std::runtime_error` (from `<stdexcept>`) with a clear message when the path is rejected.

## Acceptance criteria

- Rejects an empty argument.
- Rejects absolute or rooted paths using `has_root_path()`. This also catches `/notes.txt` on Windows, where `is_absolute()` is false.
- Normalises with `lexically_normal()`, so `docs/./notes.txt` becomes `docs/notes.txt`, then rejects a result that is `.` or empty.
- Rejects a path that leaves the project: after normalising, its first part is `..` (for example `../a.txt` or `docs/../../a.txt`).
- Rejects any path with a part equal to `.git` or `.minigit`, found by looping over the path's parts.
- Returns the result with `generic_string()`, so the stored path uses `/` on every platform.
- Does not touch the disk.
- Local CTest and Windows/Ubuntu CI pass.

## Tests to write

- Accepted: `notes.txt`, `docs/notes.txt`, `docs/./notes.txt` -> `docs/notes.txt`, `docs/../notes.txt` -> `notes.txt`, `my..notes.txt`.
- Rejected (check that `std::runtime_error` is thrown): `""`, `.`, `/notes.txt`, `../notes.txt`, `docs/../../notes.txt`, `.minigit/HEAD`, `.git/config`, `docs/.minigit/x`.

## Out of scope

Symbolic links (RP-6); checking whether the file exists.

## Reviewer focus

Does every rule have a test? Does any check search the string for `".."` instead of looking at the path's parts? (That wrongly rejects a file named `my..notes.txt`.)
