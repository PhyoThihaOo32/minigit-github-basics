# Issue RP-4 Add commitDirectory(id) and committedFile(id, path)

## Goal

Return where one commit, and one file inside that commit, are stored.

## Dependency

RP-1 merged.

## Functions to add

- `std::filesystem::path commitDirectory(const std::string& id) const;` returns `metadataRoot_ / "commits" / id`
- `std::filesystem::path committedFile(const std::string& id, const std::string& path) const;` returns `commitDirectory(id) / "files" / path`

## Acceptance criteria

- `committedFile` reuses `commitDirectory` instead of repeating `"commits"`.
- Nothing is created on disk.
- Local CTest and Windows/Ubuntu CI pass.

## Tests to write

- `commitDirectory("3")` equals `root / ".minigit" / "commits" / "3"`.
- `committedFile("3", "docs/notes.txt")` equals `root / ".minigit" / "commits" / "3" / "files" / "docs" / "notes.txt"`.
- `committedFile("3", "a.txt").parent_path()` equals `commitDirectory("3") / "files"`.

## Out of scope

Reading HEAD, choosing commit IDs, writing commit metadata.

## Reviewer focus

Is there exactly one place that knows the word `"commits"`?
