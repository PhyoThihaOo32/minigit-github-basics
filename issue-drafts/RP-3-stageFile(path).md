# Issue RP-3 Add stageFile(path)

## Goal

Return where the staged copy of a project file is stored.

## Dependency

RP-1 merged.

## Functions to add

- `std::filesystem::path stageFile(const std::string& path) const;` returns `metadataRoot_ / "stage" / path`

The argument is a project-relative path such as `"notes.txt"` or `"docs/notes.txt"`.

## Acceptance criteria

- Works for a file at the top of the project and for a file in a sub-folder.
- Does not create the stage folder or the file.
- Local CTest and Windows/Ubuntu CI pass.

## Tests to write

- `stageFile("notes.txt")` equals `root / ".minigit" / "stage" / "notes.txt"`.
- `stageFile("docs/notes.txt")` equals `root / ".minigit" / "stage" / "docs" / "notes.txt"`. This passes on Windows too, because `path` compares parts, not characters.
- `stageFile("docs/notes.txt").filename()` equals `"notes.txt"`.

## Out of scope

Checking whether the argument is safe (RP-5); copying files.

## Reviewer focus

Does a test cover a sub-folder? Is the path built with `/` rather than `+`?
