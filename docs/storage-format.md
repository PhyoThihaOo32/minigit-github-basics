# MiniGit storage format

This is the shared interface and storage proposal for HW12. Review it with the
class headers before implementing the remaining classes. `MiniGitManager`,
`init()` and `RepositoryPaths::metadataRoot()` are the names used by this team.

## Fresh repository

Running `minigit init` in a project without `.minigit` creates exactly:

```text
.minigit/
├── HEAD
├── index.txt
├── next-id.txt
├── stage/
└── commits/
```

| Location | Initial contents | Purpose |
| --- | --- | --- |
| `HEAD` | Empty file, zero bytes | Latest successful commit ID; empty means no commits |
| `index.txt` | Empty file, zero bytes | Paths selected for the next commit |
| `next-id.txt` | `1\n` | Next available positive decimal commit ID |
| `stage/` | Empty directory | Selected file copies, preserving relative folders |
| `commits/` | Empty directory | Complete saved snapshots and their records |

In the table, `\n` means one LF byte. Metadata writers use LF on all platforms.
Readers accept LF and CRLF line endings and a missing final newline. Empty HEAD
and index files contain no newline. Working files and staged copies retain their
original bytes; this metadata convention does not convert user-file contents.

## Initialization and errors

Inspect the project metadata before writing anything:

1. If `.minigit` is absent, create the fresh layout. Check every filesystem
   operation and every `WriteResult` returned by `FileSystem`.
2. If `.minigit` exists and passes the checks below, print an already-initialized
   message and return successfully without writing or removing anything.
3. If `.minigit` exists but is damaged, report an error. Do not repair, truncate,
   replace or remove existing metadata automatically.

For M2, an existing repository must have:

- A real `.minigit` directory and real `stage` and `commits` directories.
- Regular, readable `HEAD`, `index.txt` and `next-id.txt` files.
- HEAD that is empty or holds one positive decimal commit ID.
- `next-id.txt` that holds one positive decimal ID, greater than HEAD when HEAD
  is nonempty.

Required metadata paths must not be symbolic links. A `.minigit` ordinary file,
missing HEAD, a directory in place of a required file, unreadable metadata, or
invalid ID text is an error. Existing selected entries, staged files, saved
commits and additional files must be preserved on repeated initialization.
The `init` check does not parse every saved commit or require future class
implementations. Full commit-history validation is outside M2.

The classroom workflow runs one command at a time and assumes other processes
do not replace metadata while a command runs. A separate status check is not an
atomic no-follow open, and this contract does not promise concurrent or hostile
filesystem safety. A failed fresh initialization may leave partial metadata;
the next init must report it as damaged. Recovery must never remove a layout
that existed before the current invocation.

`MiniGitManager::init()` returns `void`. It prints a success message after new
initialization, or an already-initialized message for a valid existing repository.
On failure it throws a useful exception. `CommandLineApp::run()` catches command
failures, prints `Error: ...` to standard error, and returns `1`; `main` returns
that code. Successful new and repeated initialization return `0`.

M2 accepts exactly `minigit init`. No command, any unsupported command (including
`minigit status` for this milestone), or extra arguments must produce `Error: ...`
and exit `1` before creating metadata. Declaring future commands in a header does
not enable them in the M2 CLI.

## M2 call chain and ownership

```text
main(argc, argv)
  -> CommandLineApp::run(argc, argv) const -> int
     -> validate arguments
     -> RepositoryPaths(current_path)
     -> FileSystem()
     -> MiniGitManager(paths, fileSystem)
        -> init()
           -> RepositoryPaths::metadataRoot(), headFile(), indexFile()
           -> FileSystem::fileStatus() and readTextFile()
           -> FileSystem::createDirectories() and writeTextFile() for a new repo
```

`CommandLineApp` keeps the paths and filesystem objects alive while the manager
uses them. Construction itself changes no files. The manager stores references
only to those completed M1 classes; constructing it must not require unfinished
IndexTable, WorkingTree, CommitStore or FileState implementations.

The added general filesystem declarations are `fileStatus(path) const`, returning
`std::filesystem::file_status`, and `createDirectories(path) const`, returning
`void`. Inspection does not follow the final symbolic link. A nonexistent path
returns `file_type::not_found`; other inspection failures and directory-creation
failures throw `std::filesystem::filesystem_error`. Existing filesystem methods
keep their current result types and signatures.

MG-1 (#56) implements the manager constructor and the agreed filesystem support;
MG-2 (#60) implements initialization. The CLI and executable integration tests
need their own M2 issues. This interface PR supplies declarations and compile
checks, not those implementations.

## Selected paths and working files

The root `index.txt` lists **selected** project-relative file paths, one per line;
it is not the inventory of every previously committed file. Store paths using
`generic_string()`, so separators are `/` on every platform. Call
`RepositoryPaths::safeRelativeFile()` before passing user paths to IndexTable or
constructing stage locations. Reject CR and LF inside an index entry because
one entry must occupy one line.

`IndexTable::readIndex()` delegates disk reading to the existing FileSystem
reader. `addPath()` preserves existing order and adds an exact entry only once.
IndexTable must reject CR/LF before writing, even if the caller has already used
`safeRelativeFile()`; the current M1 path validator does not reject line breaks.

The existing `FileSystem::readIndex()` takes a non-const path reference. Inside
`IndexTable::readIndex() const`, use a local copy so the stored path stays const:

```cpp
auto indexPath = indexPath_;
return fileSystem_.readIndex(indexPath);
```

Keep the M1 filesystem API unchanged; do not make the stored path mutable or
cast away constness. This is guidance for the IT-2 implementation.

`removePath()` removes one exact selection; its implementation needs a later
issue. `clearIndex()` leaves an empty index file after successful commit cleanup.
All operations preserve working-file contents.

`WorkingTree::listFiles()` returns ordinary project-relative file paths using
`/`, excluding `.git` and `.minigit`. It does not copy, remove or modify files.

## Saved commits for M4

```text
.minigit/commits/<id>/
├── files/
│   └── <project-relative paths>
├── parent.txt
├── message.txt
├── time.txt
└── index.txt
```

- `files/` contains a complete snapshot: selected staged versions plus unchanged
  files from the previous snapshot. It is not just the newly selected files.
- `parent.txt` is empty for the first commit and otherwise holds the parent ID.
- `message.txt` contains the supplied commit message.
- `time.txt` records UTC as `YYYY-MM-DDTHH:MM:SSZ`, followed by LF.
- The commit's `index.txt` lists all files in that saved snapshot, one per line.

IDs are positive decimal integers starting at `1`. HEAD and parent IDs may be
stored without a trailing newline, matching the existing `updateHEAD` writer;
readers also accept one final LF or CRLF. Write the next-ID counter with a final
LF. `CommitInfo` returns the ID, parent ID, message and time as strings.

`CommitStore::saveCommit(selectedPaths, message)` saves the full snapshot and
record, advances the next-ID counter, and returns the saved ID. It does not
update HEAD or clear stage/index. Only after saving succeeds does the manager
update HEAD and clear successfully committed stage selections and the root
index. Do not move away staged data before it has been saved successfully.
Commit failure recovery and ID reservation are part of the M4 implementation;
declaring these methods does not claim that multi-file operations are atomic.

Before building any storage path, CommitStore must validate selected paths using
the relative-path rules above and validate IDs as positive decimal digits only.
It must fail without overwriting an already occupied commit directory, including
one left by an interrupted save. Never infer that an available counter means the
corresponding directory is absent. A saved record whose HEAD update fails, or
cleanup that fails after HEAD advances, must be reported rather than silently
retried as a new commit; recovery is a separate M4 design decision. Gaps in IDs
are allowed. `init` does not delete orphaned records or recover pending cleanup.

The planned snapshot operation copies selected files. `removePath()` unselects
a path; it does not record deletion from a committed snapshot. Persisting a
deletion needs a separate agreed representation and issue before implementation.

`getHistory()` follows HEAD and parent IDs from newest to oldest.
`getCommitInfo(id)` returns one record, and `getCommittedFiles(id)` returns the
snapshot's relative file paths. Missing or unreadable requested records must be
reported rather than treated as empty successful commits.

## File state for M5

`FileInfo` contains `exists` and `text`. This distinguishes a missing file from
an existing empty file. `FileState::classify()` receives working, staged and
committed information plus `selected` (whether the root index lists this path).
It performs no filesystem operations and may return multiple status strings, so
staged content can also be modified again in the working tree. Use only these
labels: `untracked`, `staged`, `modified`, `deleted`, and `unchanged`. Return
`unchanged` only when no other label applies. A selected path with missing staged
contents must report an error instead of inventing a status label. ST-2 defines
the full classification matrix and tests before callers rely on those results.

## Build and review

With `BUILD_TESTING` enabled, CMake compiles each remaining class header in its
own translation unit through `interface_headers_check`. It does not instantiate
future classes or link undefined method bodies. The existing six CTest suites
must continue passing on macOS locally and Windows/Ubuntu CI.

Runtime tests belong with each implementation. M2's final integration test must
launch the real executable in owned temporary directories and check all five
HW12 cases, exit codes, error output, initial bytes, repeated-run preservation,
and cleanup. M2 is complete only when those implementations are reviewed,
merged, and green on main.
