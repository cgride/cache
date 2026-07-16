# Cgride Cache

`cgride-cache` provides the local cache metadata layer for Cgride.

It is responsible for stable cache keys, file signatures, cache entries, cache options, and a small filesystem-backed cache store. The module does not know about projects, targets, compilers, build graphs, or execution. Higher-level modules decide what should be cached; this module only provides the primitives needed to record and query that information safely.

## Purpose

Cgride is designed around a modular build engine. The cache module exists so the engine can later decide whether a task is still valid without mixing cache logic into the executor, the graph, the project model, or the toolchain layer.

A cache entry records a key, the input file signatures observed when the entry was created, and the output paths produced by the operation. The store writes this metadata under a local cache root.

The current implementation focuses on metadata. It does not yet copy or restore output files from an object store. That behavior belongs to a later stage once the engine can connect graph tasks, commands, outputs, and cache decisions.

## Module boundary

This module depends only on:

- `cgride::core`

It must not depend on:

- `cgride::project`
- `cgride::graph`
- `cgride::toolchains`
- `cgride::executor`
- `cgride::engine`
- `cgride::config`
- `cgride::cli`

This keeps the cache usable by the engine without making it responsible for build planning or command execution.

## Main components

### CacheKey

`CacheKey` is a stable string-backed identifier for one cache entry.

It can be created directly from a string or from multiple stable parts:

```cpp
auto key = cgride::cache::CacheKey::from_parts({
    "compile",
    "src/main.cpp",
    "c++23",
    "debug",
});
```

The key is comparable and hashable for in-memory containers. It also provides a filesystem-safe name for storing metadata files.

### FileSignature

`FileSignature` describes the observable state of one file:

- path
- size
- last write time
- optional content hash

It can be created from an existing regular file:

```cpp
auto signature = cgride::cache::FileSignature::from_file(
    "src/main.cpp",
    true
);
```

The second argument controls whether the file contents should be hashed.

### CacheEntry

`CacheEntry` stores the metadata for one cached operation:

- cache key
- input file signatures
- output paths
- producer label
- creation time
- last used time
- hit count

Example:

```cpp
cgride::cache::CacheEntry entry(
    cgride::cache::CacheKey::from_string("compile:main.cpp")
);

entry
    .producer("compile:main.cpp")
    .input(signature.value())
    .output("build/main.o");
```

### CacheOptions

`CacheOptions` controls where the cache lives and how entries are verified.

Default cache root:

```text
.cgride/cache
```

Default behavior:

- cache enabled
- output verification enabled
- input content hashing disabled
- missing cache directories created automatically

### CacheStore

`CacheStore` stores and reads cache entry metadata from the filesystem.

The layout is intentionally simple:

```text
.cgride/cache/
├── entries/
│   └── <cache-key>.entry
└── objects/
    └── <cache-key>/
```

At this stage, the `entries` directory stores metadata. The `objects` directory is reserved for future output storage.

## Example

```cpp
#include <cgride/cache/cache_store.hpp>

int main()
{
  cgride::cache::CacheOptions options(".cgride/cache");
  cgride::cache::CacheStore store(options);

  auto initialized = store.initialize();

  if (!initialized)
  {
    return 1;
  }

  auto key = cgride::cache::CacheKey::from_string("compile:main.cpp");

  cgride::cache::CacheEntry entry(key);

  entry
      .producer("compile:main.cpp")
      .output("build/main.o");

  auto stored = store.put(entry);

  if (!stored)
  {
    return 1;
  }

  if (store.contains(key))
  {
    auto loaded = store.get(key);

    if (!loaded)
    {
      return 1;
    }
  }

  return 0;
}
```

## Build

From the module directory, use the Vix workflow:

```bash
vix build
```

For a release build:

```bash
vix build --preset release
```

## Run tests

```bash
vix check --tests
```

Or run the test command directly:

```bash
vix tests
```

## Install

```bash
vix install
```

The install step exposes the `cgride::cache` integration target, public headers, and package metadata.

