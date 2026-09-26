# Architecture

## Layered Design

```
┌─────────────┐
│   CLI/Main  │  main.cpp — argument parsing, command dispatch
├─────────────┤
│  Commands   │  init, hash-object, cat-object, add, status, write-tree, commit, log, revert, help
├─────────────┤
│ Repository  │  .gitx/ management, HEAD, index loading/saving
├─────────────┤
│ ObjectStore │  Content-addressed read/write with SHA-256 keys
├─────────────┤
│   Objects   │  Blob, Tree, Commit — serialize/deserialize
├─────────────┤
│   Index     │  Staging area: path → blob_id + metadata
└─────────────┘
```

## File Layout

```
include/gitx/
  types.hpp              ObjectId, ObjectType, to_bytes/from_bytes helpers
  hash.hpp               SHA-256 Hash utility
  object.hpp             Object base class (virtual serialize)
  blob.hpp               Blob — raw content
  tree.hpp               TreeEntry, Tree — sorted directory entries
  commit.hpp             Commit — tree + parent + author + message
  object_store.hpp       ObjectStore — sharded file storage
  index.hpp              Index — staging area with binary serialization
  repository.hpp         Repository — ties everything together
  utils/
    file_utils.hpp       File I/O helpers
    time_utils.hpp       ISO-8601 timestamps
    serialization.hpp    Big-endian integer/string serialization

src/
  main.cpp               CLI entry point
  hash.cpp               Self-contained SHA-256 implementation (no OpenSSL)
  blob.cpp, tree.cpp, commit.cpp    Object implementations
  object_store.cpp       Sharded object storage
  index.cpp              Index binary format read/write
  repository.cpp         Repo discovery, init, HEAD management
  commands/*.cpp         One file per command
  utils/*.cpp            Utility implementations
```

## Key Design Decisions

1. **No external dependencies** — SHA-256 implemented inline (~100 lines) for zero-setup builds.
2. **std::filesystem** — Portable path handling across Windows/Linux/macOS.
3. **Binary index format** — Versioned, corruption-detectable, extensible. Not Git-compatible.
4. **Uncompressed objects** — Simplicity over performance; compression layer can be added later.
5. **text-based commit format** — Human-readable, easy to parse, deterministic serialization.
