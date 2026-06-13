# FileSystemManager

![CI](https://github.com/Filippov-Nikolay/FileSystemManager/actions/workflows/ci.yml/badge.svg)

A console-based file manager with multi-user authentication, written in C++17 with **zero external dependencies**.

Users are isolated inside a sandbox `workspace/` directory — no operation can escape it. The CLI features tab autocomplete, persistent command history, and password masking.

---

## Demo

```
FileSystemManager.exe

1. Login
2. Register
3. Exit
Choose option: 2
First name: Alice
Last name: Smith
Username: alice
Password: *****
User registered successfully.

Choose option: 1
Username: alice
Password: *****
Login successful. Welcome, Alice!

File manager started. Type 'help' to show available commands.

alice@workspace> crdr projects
Directory created successfully.

alice@workspace> cd projects
Directory changed to: workspace\projects

alice@workspace\projects> crf notes.txt
File created successfully.

alice@workspace\projects> write notes.txt Meeting at 9am
File written successfully.

alice@workspace\projects> cat notes.txt
Meeting at 9am

alice@workspace\projects> append notes.txt Bring laptop
Content appended successfully.

alice@workspace\projects> lsdr
[FILE]  notes.txt

alice@workspace\projects> cd
Directory changed to: workspace

alice@workspace> szdr projects
Directory size: 32 bytes

alice@workspace> shmsk *.txt
...\workspace\projects\notes.txt

alice@workspace> rmf projects\notes.txt
Delete file 'projects\notes.txt'? [y/N] y

File deleted successfully.

alice@workspace> clfm
File manager closed.
```

---

## Features

| Feature | Details |
|---|---|
| Authentication | Register / login with SHA-256 + random salt password hashing |
| Sandbox | All FS operations confined to `workspace/`; traversal attacks blocked |
| Tab autocomplete | Commands on first token, filesystem paths on arguments; case-insensitive |
| Command history | ↑/↓ navigation; persisted to `history.txt` across sessions |
| Password masking | Input echoed as `*`; Backspace works; extended keys skipped |
| 22 commands | Create, rename, copy, move, delete, list, size, read, write, append, search |

---

## Architecture

```
┌─────────────────────────────────────────┐
│               Application               │  ← orchestrator (app/)
│   RunAuthLoop()   RunFileManagerLoop()  │
└────────┬──────────────────┬─────────────┘
         │                  │
┌────────▼───────┐  ┌───────▼───────────────────────┐
│   auth/        │  │   file_system/                │
│  AuthService   │  │  CommandDispatcher            │
│  UserRepository│  │    └─ unordered_map handlers  │
│  PasswordHasher│  │  CommandParser  (tokenizer)   │
└────────────────┘  │  FileManager    (FS + sandbox)│
                    └───────────────────────────────┘
         │                  │
┌────────▼──────────────────▼────────────┐
│   logging/Logger     utils/ReadLine    │  ← shared infrastructure
│   (RAII ofstream)    (Tab + history)   │
└────────────────────────────────────────┘
```

**Dependency rule:** every arrow points downward. No circular dependencies. Each layer only knows about layers below it.

---

## Commands

### Directories

| Command | Arguments | Description |
|---|---|---|
| `crdr` | `<path>` | Create directory |
| `rndr` | `<from> <to>` | Rename directory |
| `cpdr` | `<from> <to>` | Copy directory (recursive) |
| `mvdr` | `<from> <to>` | Move directory |
| `rmdr` | `<path>` | Delete directory (asks y/N) |
| `lsdr` | `[path]` | List directory contents (default: current) |
| `szdr` | `[path]` | Show total directory size (default: current) |

### Files

| Command | Arguments | Description |
|---|---|---|
| `crf` | `<path>` | Create empty file |
| `rnf` | `<from> <to>` | Rename file |
| `cpf` | `<from> <to>` | Copy file |
| `mvf` | `<from> <to>` | Move file |
| `rmf` | `<path>` | Delete file (asks y/N) |
| `szf` | `<path>` | Show file size in bytes |
| `cat` | `<path>` | Print file contents |
| `write` | `<path> <text>` | Write one line to file (overwrites) |
| `append` | `<path> <text>` | Append one line to file |

### Navigation

| Command | Arguments | Description |
|---|---|---|
| `pwd` | — | Print current directory |
| `cd` | `[path]` | Change directory (no arg → workspace root) |
| `shmsk` | `[path] <mask>` | Recursive glob search (`*`, `?` wildcards, e.g. `*.txt`, `notes_?.md`) |

### System

| Command | Description |
|---|---|
| `help` | Show all commands |
| `clear` | Clear the screen |
| `clfm` | Exit file manager |

---

## Build

**Requirements:** Visual Studio 2017 or newer, C++17, Windows 10+

1. Open `FileSystemManager.sln`
2. Select configuration: `Debug` or `Release`, platform `x64`
3. Build → `Ctrl+Shift+B`

The executable is written to `x64\Debug\` or `x64\Release\`. Run it from the project root so that `users.txt`, `logs.txt`, and `history.txt` are created in the same directory as the `.sln`.

No CMake, no vcpkg, no NuGet packages.

---

## Design Decisions

### SHA-256 from scratch
The standard library has no hashing. Rather than pulling in OpenSSL or Botan for a single function, SHA-256 was implemented in ~90 lines following FIPS 180-4. All helpers (`BigSigma`, `SmSigma`, `Ch`, `Maj`) are `constexpr`; `ProcessBlock` lives in an anonymous namespace. This keeps the build self-contained and demonstrates understanding of the algorithm.

### `std::unordered_map` command dispatch
The original `if-else if` chain grew linearly with each command. The map-based dispatcher (`CommandDispatcher`) registers all 22 handlers in `RegisterCommands()` as lambdas. Adding a new command is a single entry. `GetCommandNames()` returns the sorted keys — the only source of truth consumed by both tab completion and `ShowHelp`.

### `ShowHelp` built from handler metadata
Each handler entry carries `usage` and `description` strings. `ShowHelp` iterates a hardcoded category list and looks up metadata from the map. There is no duplicated command listing — the description lives exactly once.

### `std::optional<User>` session
`RunAuthLoop()` returns `std::optional<User>`. The caller checks with `if (!user)` rather than a boolean flag plus a separate object. `nullopt` means "exit" — no sentinel values.

### Directory traversal protection via path iterators
`ResolvePath` calls `fs::weakly_canonical` then compares the result against `rootPath` component-by-component using `path::iterator`. A string-based check (e.g. `find("/workspace")`) would allow `../workspace2` to pass. The iterator comparison is immune to this class of bypass.

### `Completer` as a typed alias
```cpp
using Completer = std::function<std::vector<std::string>(const std::string& prefix,
                                                          const std::string& linePrefix)>;
```
`linePrefix` lets `ReadLine` stay generic — it doesn't know what a "command" is. The caller in `Application` decides: empty prefix → command names, non-empty → filesystem paths.

---

## Security Notes

**Passwords are hashed, not stored in plaintext.** Each password is salted with 16 random bytes before SHA-256 hashing. The stored entry has the form `<hex-salt>:<hex-hash>`. Legacy entries without a salt delimiter are handled transparently by the `Verify` fallback.

**Known limitations (acceptable for a portfolio project):**

- SHA-256 is a fast hash. In production, use PBKDF2, bcrypt, or Argon2 to make brute-force attacks computationally expensive. This project avoids external libraries by design.
- `users.txt` has no locking. Concurrent processes writing simultaneously can corrupt the file.
- The salt is generated with `std::mt19937` seeded from `std::random_device`. On most platforms `random_device` provides cryptographic-quality entropy; on some older MinGW builds it does not. For production use `BCryptGenRandom` (Windows) or `/dev/urandom` directly.

---

## Project Structure

```
├── README.md
├── FileSystemManager.sln
├── FileSystemManager/
│   ├── FileSystemManager.vcxproj
│   ├── commands.md
│   └── src/
│       ├── main.cpp
│       ├── app/
│       │   ├── Application.h
│       │   └── Application.cpp
│       ├── auth/
│       │   ├── User.h
│       │   ├── AuthService.h / .cpp
│       │   ├── UserRepository.h / .cpp
│       │   └── PasswordHasher.h / .cpp
│       ├── file_system/
│       │   ├── Command.h
│       │   ├── CommandParser.h / .cpp
│       │   ├── CommandDispatcher.h / .cpp
│       │   └── FileManager.h / .cpp
│       ├── logging/
│       │   └── Logger.h / .cpp
│       └── utils/
│           └── ReadLine.h / .cpp
└── FileSystemManagerTests/
    ├── FileSystemManagerTests.vcxproj
    └── src/
        ├── main.cpp
        └── tests/
            ├── TestFramework.h
            ├── CommandParserTests.h
            ├── FileManagerTests.h
            ├── PasswordHasherTests.h
            └── UserRepositoryTests.h
```
