# GitX

A Git-like local version control system built from scratch in C++20.

GitX implements core Git concepts—including content-addressable storage, blobs, trees, commits, a staging index, and history traversal—without using Git's source code or libraries.

The goal is to build a small, readable implementation that makes Git's internal architecture easier to understand.

---

## Features

- SHA-256 content-addressable object storage
- Immutable blob, tree, and commit objects
- Binary staging index
- Tree-based directory snapshots
- Parent-linked commit history
- Working-tree status detection
- Full-tree revert
- Deterministic object serialization
- C++20 implementation
- CMake build system

---

## Architecture

```
CLI (main.cpp)
      |
      v
Command Handlers
      |
      v
Repository API
      |
      +-------------------+
      |                   |
      v                   v
 ObjectStore            Index
      |
      v
+---------------------------+
| Blob | Tree | Commit      |
+---------------------------+
```

### Components

| Component | Description |
|-----------|-------------|
| **CLI Layer** | Parses commands and arguments and dispatches them to command handlers. |
| **Command Layer** | Implements commands such as `init`, `add`, `commit`, `status`, and `log`. |
| **Repository Layer** | Manages the `.gitx/` directory, HEAD, index, and object store. |
| **Object Model** | Blob, Tree, and Commit inherit from Object and provide serialization. |
| **ObjectStore** | Handles SHA-256 content-addressed object storage. |
| **Index** | Stores staged file paths, blob IDs, and file metadata in a binary format. |

---

## Object Model

### Content-Addressable Storage

Every GitX object is identified by a SHA-256 hash:

```
id = SHA-256(type + "\0" + serialized_content)
```

The same content always produces the same object ID.

Objects are stored using the first two characters of the hash as a directory:

```
.gitx/
└── objects/
    └── ab/
        └── cdef123456...
```

This gives GitX immutable, content-addressable object storage.

### Blob

A blob stores file contents only. It does not contain the filename.

Therefore, two files with identical contents can reference the same blob object:

```
hello.txt ──┐
            ├──> Blob
 copy.txt ──┘
```

This separates file contents from filesystem names and locations.

### Tree

A tree represents a directory snapshot. Each entry contains:

```
(mode, type, name, object_id)
```

For example:

```
100644 blob abc123 README.md
100644 blob def456 main.cpp
040000 tree ghi789 src
```

Tree entries are sorted lexicographically by name before serialization. This ensures deterministic tree hashes.

Trees can contain other trees, allowing complete directory hierarchies to be represented recursively.

### Commit

A commit records a tree snapshot and, except for the root commit, its parent.

The serialized format is:

```
tree <tree_hash>
parent <parent_hash>
author <author>
timestamp <ISO8601>

<message>
```

The `parent` line is omitted for the root commit.

Commits therefore form a linked history:

```
Commit C
   |
   v
Commit B
   |
   v
Commit A
   |
   v
Root Commit
```

---

## Index

The index is GitX's staging area. It is stored at:

```
.gitx/index
```

The index uses a binary format containing:

- Version header (`GITX_INDEX_VERSION = 1`)
- Blob ID
- File size
- Modification time
- Length-prefixed file path

Each entry maps a working-tree path to the blob that will be included in the next tree.

The basic workflow is:

```
Working Tree
     |
     | gitx add
     v
   Index
     |
     | gitx commit
     v
   Tree
     |
     v
  Commit
```

---

## HEAD

The current commit is stored in:

```
.gitx/HEAD
```

Before the first commit, HEAD is empty. After committing, it contains the current commit's SHA-256 object ID.

GitX V1 does not implement branches, so HEAD directly references a commit.

---

## Status Algorithm

`gitx status` compares the working tree, index, and current commit.

The algorithm is:

1. Collect working-tree files, excluding `.gitx/`.
2. Load index entries.
3. If HEAD exists, load the referenced commit.
4. Resolve the commit's tree and tracked entries.
5. Re-hash indexed files on disk.
6. Compare the working-tree state with the index.
7. Identify staged, modified, and untracked files.

The result can identify:

| Status | Description |
|--------|-------------|
| **Staged** | The file is represented in the index and is ready to be committed. |
| **Modified / Not staged** | The working-tree contents differ from the indexed blob. |
| **Untracked** | The file exists in the working tree but is not present in the index. |

---

## Commit Flow

Creating a commit follows this process:

```
gitx commit -m "message"
          |
          v
    Parse message
          |
          v
   Build tree from index
          |
          v
       Read HEAD
          |
          v
  Create Commit object
          |
          v
  Hash and store commit
          |
          v
      Update HEAD
```

### Steps

1. Parse the `-m` message.
2. Return an error if the message is missing.
3. Build a tree from the current index.
4. Read HEAD to determine the parent commit.
5. Create a Commit object.
6. Serialize and hash the commit.
7. Write the commit to the object store.
8. Update HEAD.

---

## Log Traversal

`gitx log` starts at HEAD and follows parent references until reaching the root commit.

```
HEAD
 |
 v
Commit C
 |
 v
Commit B
 |
 v
Commit A
 |
 v
Root
```

The algorithm is intentionally simple:

1. Read HEAD.
2. Load the referenced commit.
3. Print the commit.
4. Read its parent.
5. Repeat until no parent exists.

---

## Build

### Requirements

- CMake 3.20+
- C++20-compatible compiler
  - GCC
  - Clang
  - MSVC

### Build from Source

```bash
cmake -S . -B build
cmake --build build
```

Run GitX:

```bash
./build/gitx help
```

---

## Usage

```bash
gitx init
gitx hash-object <file>
gitx cat-object <id>
gitx add <path>
gitx status
gitx write-tree
gitx commit -m "<message>"
gitx log
gitx revert <commit_hash>
gitx help
```

### Command Reference

| Command | Description |
|---------|-------------|
| `gitx init` | Create a `.gitx/` repository |
| `gitx hash-object <file>` | Hash a file as a blob |
| `gitx cat-object <id>` | Display an object's contents |
| `gitx add <path>` | Stage files |
| `gitx status` | Show working-tree status |
| `gitx write-tree` | Build a tree from the index |
| `gitx commit -m "<message>"` | Create a commit |
| `gitx log` | Display commit history |
| `gitx revert <commit_hash>` | Restore the working tree to a previous commit |
| `gitx help` | Display available commands |

---

## End-to-End Example

### 1. Initialize a Repository

```bash
gitx init
```

### 2. Create a File

```bash
echo "Hello World" > hello.txt
```

### 3. Check Its Status

```bash
gitx status
```

The file should appear as untracked.

### 4. Stage the File

```bash
gitx add hello.txt
```

Check the status again:

```bash
gitx status
```

The file should now appear as staged.

### 5. Build the Tree

```bash
gitx write-tree
```

### 6. Create the First Commit

```bash
gitx commit -m "Initial commit"
```

### 7. View the History

```bash
gitx log
```

### 8. Modify the File

```bash
echo "Hello GitX" > hello.txt
```

Check the changes:

```bash
gitx status
```

### 9. Stage and Commit the Update

```bash
gitx add hello.txt
gitx commit -m "Update greeting"
```

### 10. View the History Again

```bash
gitx log
```

The second commit should reference the first commit as its parent.

---

## Limitations

GitX V1 intentionally implements only a subset of Git's functionality.

Currently unsupported:

- Branches
- Checkout
- Merge
- Diff
- Stash
- Tags
- Reflog
- Remote repositories
- Push
- Pull
- Fetch
- Clone
- Rename detection
- Compression
- Pack files
- Garbage collection
- Path-scoped revert

Revert currently operates on the entire tree.

Objects are stored uncompressed to keep the implementation simple and easy to inspect.

---

## Roadmap

### V2

- Diff
- Branches
- Checkout
- Improved revert
- Better working-tree comparison

### V3

- Merge
- Stash
- Tags
- Reflog

### V4

- Remote repositories
- Push
- Pull
- Fetch
- Clone

### V5

- Commit graph viewer
- Diff viewer
- History visualization
- Object compression
- Pack files
- Garbage collection
- Faster object traversal

---

## Design Goals

GitX is built around four main principles:

| Principle | Description |
|-----------|-------------|
| **Readable** | Keep the implementation understandable. |
| **Minimal** | Implement the essential concepts without unnecessary complexity. |
| **Deterministic** | Ensure stable serialization and hashing. |
| **Educational** | Make Git's internal architecture visible. |

---

## Project Status

GitX V1 is an educational implementation of the fundamental mechanics behind a version control system.

The core lifecycle is:

```
Files
  |
  v
Blobs
  |
  v
Index
  |
  v
Tree
  |
  v
Commit
  |
  v
HEAD
  |
  v
History
```

The project is intentionally small enough to study while implementing the essential concepts behind Git's object model.

---

## License

Educational project — no license specified.
