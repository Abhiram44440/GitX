# Index (Staging Area)

## Purpose

The index is an intermediate staging area between the working tree and the next commit. When you `gitx add <file>`, the file's content is hashed as a blob and recorded in the index.

## Format

Binary format at `.gitx/index`:

```
┌─────────────────────────────────┐
│ Version (4 bytes, big-endian)   │  GITX_INDEX_VERSION = 1
│ Entry count (4 bytes)           │
├─────────────────────────────────┤
│ Entry 0:                        │
│   blob_id (64 bytes, ASCII hex) │
│   file_size (4 bytes)           │
│   mtime (8 bytes)               │
│   path (length-prefixed string) │
├─────────────────────────────────┤
│ Entry 1:                        │
│   ...                           │
└─────────────────────────────────┘
```

## Operations

- **Add:** Hash file → write blob to store → add entry to index → save index
- **Remove:** Delete entry from in-memory map → save index
- **Write Tree:** Build Tree objects from index entries (grouped by directory)
- **Status:** Compare HEAD tree ↔ index ↔ working tree

## Persistence

The index is loaded from disk at the start of operations that need it (`add`, `status`, `write-tree`, `commit`) and saved back after modifications.

## Corrupt Detection

- Version number check on load
- Fixed-size fields ensure structural integrity
- Entry count must match actual entries
