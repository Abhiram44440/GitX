# GitX Workflow — Step by Step

This document walks through every GitX command with a real multi-file
example, showing exactly what happens inside the `.gitx/` directory.

---

## Setup

```bash
# Build GitX (one-time)
cd E:/projects/gitx
cmake -S . -B build && cmake --build build

# Create a fresh project folder
mkdir E:/gitx-demo && cd E:/gitx-demo
G="E:/projects/gitx/build/Debug/gitx.exe"
```

---

## Step 1 — `gitx init`

**Command:**
```bash
$G init
```

**Output:**
```
Initialized empty GitX repository in E:\gitx-demo\.gitx
```

**What happened inside `.gitx/`:**
```
.gitx/
├── objects/          # Empty — will hold blobs, trees, commits
├── refs/heads/       # Empty — will hold branch refs (V2)
├── HEAD              # Empty — no commits yet
├── config            # Empty — repo settings placeholder
└── index             # Doesn't exist yet — created when you first add files
```

- **`HEAD`** is empty because there's no commit history yet.
- **`objects/`** is where all blobs, trees, and commits will be stored.
- **`index`** doesn't exist — it's created the first time you run `gitx add`.

---

## Step 2 — Create Some Files

```bash
echo "Hello World" > hello.txt
echo "int main() { return 0; }" > main.cpp
echo "# My Library" > lib.hpp
mkdir src
echo "implementation" > src/utils.cpp
echo "More utils" > src/helper.cpp
```

**Working tree now:**
```
gitx-demo/
├── hello.txt
├── lib.hpp
├── main.cpp
└── src/
    ├── helper.cpp
    └── utils.cpp
```

At this point, GitX knows nothing about these files.

---

## Step 3 — `gitx status` (before adding anything)

**Command:**
```bash
$G status
```

**Output:**
```
Untracked files:
  (use "gitx add <file>" to include in what will be committed)
    hello.txt
    lib.hpp
    main.cpp
    src\helper.cpp
    src\utils.cpp
```

**How GitX determines this:**

1. Loads the index → empty (no `.gitx/index` file yet).
2. Scans the working tree → finds 5 files.
3. Compares: every file is "not in index" → classified as **untracked**.
4. HEAD is empty → no commit history to compare against.

---

## Step 4 — `gitx add hello.txt`

**Command:**
```bash
$G add hello.txt
```

**What happened internally (4 sub-steps):**

```
Working Tree                    Object Store                 Index
─────────────                   ────────────                 ─────
hello.txt ──read bytes──►  Blob("Hello World")
                                    │
                                    ▼
                           SHA256("blob\0Hello World")
                                    │
                                    ▼
                           .gitx/objects/a1/b2c3... ◄── stored
                                    │
                                    ▼
                           Entry added to index:
                           path: "hello.txt"
                           blob_id: "a1b2c3..."
                           file_size: 12
                           mtime: <timestamp>
```

**Step by step:**

1. **Read file** — `hello.txt` contents read into memory: `Hello World\n`
2. **Create Blob** — Wrap content in a `Blob` object
3. **Hash** — Compute `SHA-256("blob\0Hello World\n")` → 64-char hex ID
4. **Store** — Write to `.gitx/objects/a1/b2c3...` (sharded by first 2 chars)
5. **Update Index** — Add entry: `hello.txt → blob_hash → file_size → mtime`
6. **Save Index** — Write `.gitx/index` binary file to disk

**Now check status:**
```bash
$G status
```

```
Changes to be committed:
  (use "gitx add <file>" to update what will be committed)
    new file:   hello.txt

Untracked files:
  (use "gitx add <file>" to include in what will be committed)
    lib.hpp
    main.cpp
    src\helper.cpp
    src\utils.cpp
```

- `hello.txt` is in the index → **staged** (ready to commit)
- Other files are not in the index → still **untracked**

---

## Step 5 — `gitx add src` (add a directory)

**Command:**
```bash
$G add src
```

**What happened internally:**

GitX detected that `src` is a directory, so it recursed:

```
src/
├── helper.cpp  ──► Blob("More utils\n")     ──► stored in objects/
└── utils.cpp   ──► Blob("implementation\n") ──► stored in objects/
```

Each file becomes its own blob. The index now has 3 entries:

```
Index after adding src:
┌──────────────────┬──────────────┬───────────┬───────┐
│ Path             │ Blob ID      │ File Size │ Mtime │
├──────────────────┼──────────────┼───────────┼───────┤
│ hello.txt        │ a1b2c3...   │ 12        │ ...   │
│ src/helper.cpp   │ d4e5f6...   │ 11        │ ...   │
│ src/utils.cpp    │ g7h8i9...   │ 15        │ ...   │
└──────────────────┴──────────────┴───────────┴───────┘
```

**Status now shows:**
```
Changes to be committed:
    new file:   hello.txt
    new file:   src\helper.cpp
    new file:   src\utils.cpp

Untracked files:
    lib.hpp
    main.cpp
```

---

## Step 6 — `gitx write-tree`

**Command:**
```bash
$G write-tree
```

**Output:**
```
bb358b5a38e70a796cf873be8190025d68dc995a52c64250d5f8835ba7b7fc35
```

**What happened internally:**

GitX builds tree objects by grouping index entries by directory:

```
Grouping by directory:

Root level (no `/` in path):
  hello.txt → BlobEntry(hello.txt, a1b2c3...)

Directory "src/":
  src/helper.cpp → BlobEntry(helper.cpp, d4e5f6...)
  src/utils.cpp  → BlobEntry(utils.cpp, g7h8i9...)
```

**Tree construction:**

```
Step 1: Build "src" subtree
  Entries (sorted):
    100644 blob d4e5f6... helper.cpp
    100644 blob g7h8i9... utils.cpp
  → Serialize → SHA256 → Store as tree object
  → Returns tree ID: x1y2z3...

Step 2: Build root tree
  Entries (sorted):
    100644 blob a1b2c3... hello.txt
    040000 tree x1y2z3... src
  → Serialize → SHA256 → Store as tree object
  → Returns root tree ID: bb358b5a...

The root tree hash depends on ALL file contents.
Change one file → different root tree hash (deterministic).
```

**Note:** `write-tree` does NOT modify HEAD or create a commit. It just builds
tree objects and prints the root hash. It's a building block for `commit`.

---

## Step 7 — `gitx commit`

**Command:**
```bash
$G commit -m "First commit with hello and src"
```

**Output:**
```
[main (root-commit) 7686925] First commit with hello and src
```

**What happened internally (5 sub-steps):**

```
Step 1: Build tree from index
  (same as write-tree above → root tree hash)

Step 2: Read HEAD as parent
  HEAD is empty → this is a ROOT commit (no parent)

Step 3: Create Commit object
  ┌─────────────────────────────────────────┐
  │ tree bb358b5a38e70a796cf873be8190025d.. │
  │ author GitX User <user@gitx.local>      │
  │ timestamp 2026-08-28T05:37:05Z          │
  │                                         │
  │ First commit with hello and src         │
  └─────────────────────────────────────────┘

Step 4: Hash + Store commit
  SHA256("commit\0<above content>")
  → 768692551c145d8cead2492454dbd59f...
  → Stored in .gitx/objects/76/869255...

Step 5: Update HEAD
  .gitx/HEAD now contains: 768692551c145d8cead2492454dbd59f...
```

**Resulting object graph:**
```
HEAD ──► Commit(7686925...)
              │
              ├── tree ──► Tree(bb358b5a...)
              │                │
              │                ├── blob a1b2c3... hello.txt
              │                └── tree x1y2z3... src/
              │                     ├── blob d4e5f6... helper.cpp
              │                     └── blob g7h8i9... utils.cpp
              │
              └── parent: (none — root commit)
```

---

## Step 8 — `gitx log`

**Command:**
```bash
$G log
```

**Output:**
```
commit 768692551c145d8cead2492454dbd59f6f84fc858f6b92a47bbb3f2b3e4e9867
Author: GitX User <user@gitx.local>
Date:   2026-08-28T05:37:05Z

    First commit with hello and src
```

**How log traversal works:**

```
1. Read HEAD → "7686925..."
2. Read commit object → parse tree, parent, author, message
3. Print commit info
4. Check for parent → none (root commit)
5. Stop.
```

---

## Step 9 — Modify a File

```bash
echo "Hello GitX" > hello.txt
```

**Nothing is stored yet** — GitX only looks at files when you run `add` or `status`.

```bash
$G status
```

**Output:**
```
Changes not staged for commit:
  (use "gitx add <file>" to update what will be committed)
    modified:   hello.txt

Untracked files:
    lib.hpp
    main.cpp
```

**How GitX detects the modification:**

```
1. Load index → has entry for hello.txt with blob_id = a1b2c3...
2. Read hello.txt from disk → "Hello GitX\n"
3. Create Blob → SHA256 → new hash = z9y8x7...
4. Compare: z9y8x7... ≠ a1b2c3... → FILE WAS MODIFIED
5. Since new hash ≠ index hash → "not staged for commit"
```

---

## Step 10 — Add + Commit the Change

```bash
$G add hello.txt
```

**What happened:**

```
1. Read hello.txt → "Hello GitX\n"
2. SHA256("blob\0Hello GitX\n") → z9y8x7...
3. Store new blob in .gitx/objects/
4. UPDATE index entry: hello.txt → z9y8x7... (was a1b2c3...)
5. Save index to disk
```

```bash
$G status
```

```
Changes to be committed:
    new file:   hello.txt     ← staged with new hash
```

```bash
$G commit -m "Update greeting to Hello GitX"
```

**What happened:**

```
1. Build tree from index (hello.txt now points to new blob)
   → New root tree hash (different from first commit's tree)

2. Read HEAD → "7686925..." (first commit)
   → This becomes the PARENT

3. Create Commit:
   ┌─────────────────────────────────────────┐
   │ tree abc123...                          │
   │ parent 7686925...    ◄── link to prev!  │
   │ author GitX User <user@gitx.local>      │
   │ timestamp 2026-08-28T05:37:05Z          │
   │                                         │
   │ Update greeting to Hello GitX           │
   └─────────────────────────────────────────┘

4. Store commit → f9cee025...

5. Update HEAD → f9cee025...
```

**Object graph after two commits:**
```
HEAD ──► Commit(f9cee02...)  "Update greeting"
              │
              ├── tree ──► Tree(abc123...)
              │                │
              │                ├── blob z9y8x7... hello.txt   ← NEW BLOB
              │                └── tree x1y2z3... src/
              │                     ├── blob d4e5f6... helper.cpp
              │                     └── blob g7h8i9... utils.cpp
              │
              └── parent ──► Commit(7686925...)  "First commit"
                                   │
                                   ├── tree ──► Tree(bb358b5a...)
                                   │                │
                                   │                ├── blob a1b2c3... hello.txt
                                   │                └── tree x1y2z3... src/
                                   │
                                   └── parent: (none)
```

---

## Step 11 — Log Shows History

```bash
$G log
```

```
commit f9cee02...  "Update greeting to Hello GitX"
commit 7686925...  "First commit with hello and src"
```

**Traversal:** HEAD → f9cee02 (print) → parent 7686925 (print) → parent: none → stop.

---

## Step 12 — Add a New File

```bash
echo "void helper() {}" > src/new_file.cpp
$G add src/new_file.cpp
$G commit -m "Add new source file"
```

**What changed in the tree:**

```
Before commit 3:              After commit 3:
src/ tree:                    src/ tree:
  helper.cpp                    helper.cpp    (same blob)
  utils.cpp                     new_file.cpp  (NEW blob)
                                utils.cpp     (same blob)
```

The `src/` tree gets a new entry. This changes the root tree hash.
A new commit is created with parent = commit 2.

**Log now:**
```
commit 7cbb30d...  "Add new source file"
commit f9cee02...  "Update greeting to Hello GitX"
commit 7686925...  "First commit with hello and src"
```

---

## Step 13 — Final Status

```bash
$G status
```

```
Untracked files:
    lib.hpp
    main.cpp
```

**Explanation:** These files were never added to the index. They exist
on disk but GitX doesn't track them yet.

---

## Key Concepts Summary

| Concept | Where | What |
|---------|-------|------|
| **Working Tree** | Your files on disk | Regular files you edit |
| **Index** | `.gitx/index` | Staging area — "what goes in the next commit" |
| **HEAD** | `.gitx/HEAD` | Points to the latest commit hash |
| **Blobs** | `.gitx/objects/` | File content, identified by SHA-256 |
| **Trees** | `.gitx/objects/` | Directory snapshots (sorted entries) |
| **Commits** | `.gitx/objects/` | Snapshot + metadata + parent link |

### The Three States of a File

```
Untracked ──gitx add──► Staged ──gitx commit──► Committed
   │                      │                         │
   │ Not in index         │ In index, not in HEAD    │ In both index and HEAD
   │                      │                         │
   ◄──── gitx status ────►                         │
```

### What Each Command Does

| Command | Reads | Writes |
|---------|-------|--------|
| `init` | — | `.gitx/` directory structure |
| `add <file>` | Working tree file | Blob + Index entry |
| `status` | Working tree + Index + HEAD | — |
| `write-tree` | Index | Tree objects |
| `commit -m` | Index + HEAD | Commit object + Update HEAD |
| `log` | Commit objects (follows parent links) | — |
| `hash-object` | Working tree file | Blob (optional) |
| `cat-object` | Object store | — |

### Deterministic Hashing

Given identical files and metadata, GitX always produces identical hashes.
This is guaranteed because:

1. Tree entries are **sorted** before serialization
2. Serialization format is **fixed** (no timestamps in tree/blob serialization)
3. SHA-256 is **deterministic** — same input always gives same output

The only non-deterministic element is the **commit timestamp** (current time).
