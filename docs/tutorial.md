# Week 1 Tutorial: Issue to Merged Pull Request

This tutorial walks through one small C++ change from a task idea to a reviewed merge. Use GitHub Desktop for branch and push actions if you are new to Git; edit and build the project in Visual Studio Code.

## Before class

1. Install Visual Studio Code, GitHub Desktop, and a C++ toolchain. On Windows, install Visual Studio 2022 with **Desktop development with C++** and CMake tools.
2. Make sure every student who needs to push or review has access to this repository. This repository is public, so students can clone and read it; contributing requires repository write access.
3. In repository Settings, allow **merge commits** if the class will demonstrate that merge method. If available, configure `main` to require a pull request, one approval, and the Windows and Ubuntu CI checks. If those protections are unavailable for the current repository/account plan, enforce the same steps as class rules.
4. Choose one small item in `TODO.md`. The four starter Issue descriptions are in `issue-drafts/`.

## 1. Create an Issue from a TODO item

`TODO.md` is a Markdown list, not a live issue tracker. It does not create GitHub Issues by itself.

1. Open the selected draft from `issue-drafts/` and copy its goal, acceptance criteria, and out-of-scope section.
2. On GitHub, open **Issues → New issue**. Give the Issue a short, action-oriented title and paste the task details.
3. Keep the task small enough for one focused code change and its tests. State what the task does not include.
4. Submit the Issue. Copy its URL or number back into `TODO.md`, for example `[Issue #12](https://github.com/OWNER/REPO/issues/12)`.

An Issue describes the intended work and how to tell when it is complete. It is not code and does not make a branch automatically.

## 2. Clone and open the project locally

1. In GitHub Desktop, choose **File → Clone repository**, select this repository, and choose a local folder.
2. Choose **Open in Visual Studio Code**. Confirm the VS Code window shows the repository folder, including `CMakeLists.txt`.
3. In GitHub Desktop, select **Current Branch → New Branch**. Name it `issue-<number>-short-name`, such as `issue-12-metadata-path`, and base it on `main`.
4. Do not develop directly on `main`. The branch keeps the student's work separate until review and merge.

## 3. Change code and tests

1. Read the Issue acceptance criteria before editing.
2. Make one small implementation change and add or update a focused test.
3. Use `std::filesystem::path` operations to join paths; avoid hard-coded slash strings.
4. Save the files. In VS Code Source Control, inspect the changed-file list and diff. Confirm there are no unrelated edits.

The starter CMake file already builds `src/PathLab.cpp` and `tests/PathLabTests.cpp`. If you only edit these files, CMake usually needs no change. If you add a new `.cpp` file or a separate test executable, update `CMakeLists.txt`; see [`cmake-and-ci.md`](cmake-and-ci.md).

## 4. Configure, build, and test locally

(1) Open **Terminal → New Terminal** in VS Code at the repository root. Run:

-----------------------------------------------------------------------------------------------------------------
VS Code on Windows
```
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/PathLab.cpp tests/PathLabTests.cpp -o path_lab_tests.exe
```

VS Code on Mac OS
```
clang++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src/PathLab.cpp tests/PathLabTests.cpp -o path_lab_tests.exe
./path_lab_tests.exe
```
-----------------------------------------------------------------------------------------------------------------
or 
(2) **use CMake**

Install CMake on Windows, run the following command in the terminal (PowerShell) in VS Code, then restart VS Code
```
winget install Kitware.CMake
```
Install CMake in Mac OS; run the following command in the terminal in VS Code 
```
brew install cmake
```
Then run the following command in the terminal in VS Code
```
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```
- Configure reads `CMakeLists.txt` and prepares a build directory.
- Build compiles the library and test program.
- CTest runs the tests registered by CMake.

Fix local failures before opening the PR. Record the commands and results in the PR description or student submission sheet. The generated `build/` folder is ignored by Git.

## 5. Commit and push the branch

In GitHub Desktop:

1. Review the changed files and diff.
2. Enter a short commit message that describes this one change, such as `feat: add metadata directory path`.
3. Click **Commit to issue-<number>-short-name**.
4. Click **Publish branch** for the first push, or **Push origin** for later commits.

Only committed changes are included in a push. Push sends the branch to GitHub; it does not merge anything into `main`.

## 6. Open a pull request

1. In GitHub Desktop, click **Preview Pull Request** or open the repository on GitHub and choose **Pull requests → New pull request**.
2. Set **base** to `main` and **compare** to the feature branch. Note: both  **base** and **compare** should refer to **your forked repo**
3. Use the PR template. Explain the goal, the implementation, local test commands/results, and anything the reviewer should focus on.
4. Link the Issue with a closing keyword in the PR description, for example `Closes #12`. This will close the Issue only when the PR is merged into the repository's default branch.
5. Create the PR. Keep it open while review and CI run.

## 7. Read GitHub Actions CI

This repository's workflow is `.github/workflows/ci.yml`. Opening or updating a PR triggers GitHub Actions on Windows and Ubuntu. Each runner configures CMake, builds the project, and runs CTest in a clean cloud environment.

Open the **Checks** tab on the PR or the repository **Actions** tab. A green result means the configured steps completed successfully on that runner. If a job fails, open it, find the first failed step, read the error, fix the cause locally, commit, and push again. CI does not replace local testing; it checks the PR on the configured cloud platforms.

Current trigger detail: pushes to feature branches alone do not start this workflow; opening/updating the PR does. Pushes to `main` also trigger it. If the instructor wants CI on every branch push, change the `push.branches` setting in the workflow.

## 8. Review with a second collaborator

The reviewer should use their own GitHub account. Do not share passwords. For a private repository, the reviewer must be invited; for this public repository, reading and commenting are open, but merging requires write access.

Ask the reviewer to:

1. Read the linked Issue and its acceptance criteria.
2. Open **Files changed** and check whether the diff is small, relevant, and complete.
3. Check the implementation, tests, error cases, and CMake registration if files were added.
4. Confirm Windows and Ubuntu CI pass. Ask questions where behavior is unclear.
5. Leave specific, respectful line comments, or submit **Approve** if ready. Use **Request changes** when a correction is needed.

The author responds to comments, updates the branch if needed, reruns local tests, commits, and pushes. The PR updates and CI runs again. The reviewer checks the follow-up and resolves conversations when satisfied.

## 9. Approve and merge

When review feedback is resolved and required CI checks pass, the reviewer submits **Approve**. For a classroom demonstration of a merge commit, select **Create a merge commit** in the merge-method menu and confirm the merge. Other available merge methods produce different histories; use the method the instructor assigned.

Do not merge while review or CI is incomplete. If the PR targets `main` and its description contains `Closes #12`, merging closes Issue #12 automatically. If the PR targets a feature branch, the closing keyword does not close the Issue at that time.

## 10. Verify the result from the author account

Refresh the repository and check:

- The merged code is on `main`.
- The commit history contains the PR merge (and, for a merge commit, the merge commit).
- The PR is in the **Closed** list and marked merged; it is still viewable.
- The linked Issue is closed if the closing keyword and default-branch conditions were met.
- The Actions run for the merged `main` commit is successful.

## Student review checklist

- [ ] I compared the diff to the Issue acceptance criteria.
- [ ] I checked the implementation and test changes.
- [ ] I checked local test evidence and Windows/Ubuntu CI.
- [ ] My review comment names a specific line or behavior and explains why.
- [ ] I responded to feedback and rechecked the updated diff.

## Common beginner questions

**Does a TODO checkbox create an Issue?** No. Create the Issue separately and link it in the TODO file.

**Does Push update `main`?** No. Push uploads the feature branch; a reviewed PR is how its changes are proposed for `main`.

**Should I change CI for every code change?** Usually no. If CMake already builds and registers the changed file/test, the existing workflow will run them. Change CI only when the build process or needed environment changes.

**What if Windows fails but Ubuntu passes?** Treat it as a real platform-specific issue. Read the Windows job log; file and directory behavior can differ between operating systems.

**Can the PR disappear after merge?** It leaves the Open list but remains available in Closed pull requests with its diff, comments, reviews, and checks.
