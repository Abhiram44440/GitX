# Object Model

## Content-Addressable Storage

Every object is identified by a SHA-256 hash of its contents:

```
id = SHA-256(type_string + "\0" + serialized_bytes)
```

This guarantees that identical content always produces the same ID, regardless of when or where it was created.

## Object Types

### Blob (`ObjectType::Blob`)

- **Purpose:** Store raw file content
- **Format:** Raw bytes (no header in serialized form)
- **Key property:** Two files with identical content produce the same blob

```
Blob("hello") → hash("blob\0hello") → abc123...
Blob("hello") → hash("blob\0hello") → abc123...  (same!)
```

### Tree (`ObjectType::Tree`)

- **Purpose:** Snapshot of a directory
- **Format:** Sorted entries, each: `mode type name\0object_id`
- **Key property:** Entries sorted lexicographically by name for determinism

```
100644 blob abc123\0README.md
100644 blob def456\0main.cpp
040000 tree ghi789\0src
```

Trees are built recursively: leaf files become Blob entries, subdirectories become nested Tree entries.

### Commit (`ObjectType::Commit`)

- **Purpose:** Record a snapshot with metadata
- **Format:** Text-based with structured fields

```
tree <tree_hash>
parent <parent_hash>        # omitted for root commit
author <author>
timestamp <ISO8601>

<message>
```

Parent linkage creates a singly-linked list (or DAG with branches) of commits.

## Hashing

```
id = SHA-256("blob\0Hello World")  → 64-char hex string
```

Implemented as a self-contained SHA-256 per FIPS 180-4 — no external crypto library needed.

## Storage Layout

Objects are stored in `.gitx/objects/` with sharding:

```
.gitx/objects/
  ab/
    cdef1234567890...     (content: "blob\0file contents")
  cd/
    1234567890abcdef...   (content: "tree\0...")
```

The first 2 hex characters form the directory, the rest form the filename. This prevents filesystem issues with millions of files in one directory.
