# Development Guide

## Prerequisites

- CMake 3.20+
- C++20 compiler (GCC 10+, Clang 12+, or MSVC 2019+)

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Project Structure

Follows the layered architecture described in `docs/architecture.md`. Key conventions:

- **Headers:** `include/gitx/*.hpp` — public interfaces
- **Sources:** `src/*.cpp` — implementations
- **Commands:** `src/commands/*.cpp` — one file per CLI command

## Adding a New Command

1. Add declaration to `include/gitx/commands/commands.hpp`
2. Create `src/commands/<name>.cpp` implementing the function
3. Add dispatch entry to `src/main.cpp`
4. Update `README.md` and `docs/`

## Adding a New Object Type

1. Add to `ObjectType` enum in `include/gitx/types.hpp`
2. Create class inheriting from `Object` in `include/gitx/<type>.hpp`
3. Implement `type()` and `serialize()`
4. Handle in `ObjectStore::read()` and `ObjectStore::write()`

## Code Style

- Modern C++20: `std::filesystem`, `std::optional`, `std::variant`
- Smart pointers, RAII, const-correct
- Error handling via exceptions at command boundaries
- MSVC-compatible: use `to_bytes()`/`from_bytes()` for `std::string` ↔ `std::vector<std::byte>` conversion

## Cross-Platform Notes

- Paths use `std::filesystem::path` for portability
- `#ifdef _WIN32` for `gmtime_s` vs `gmtime_r`
- `to_bytes()`/`from_bytes()` helpers avoid MSVC's strict `std::byte` conversion rules
