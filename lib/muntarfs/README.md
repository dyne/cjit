# muntarfs

`muntarfs` is a cross-platform filesystem bundler for C/C++

It has two jobs:

1. pack a directory tree into a `.tar.gz` bundle suitable for embedding in C code
2. unpack an embedded `.tar` or `.tar.gz` bundle into a destination path at runtime

## Scope

`muntarfs` is intentionally small.

Public responsibilities:

- create a tar.gz bundle from a directory tree at build time
- optionally emit a C array from that bundle for hard-coding
- extract a tar bundle to a destination directory at runtime
- extract a tar.gz bundle to a destination directory at runtime

## API

- `muntarfs_extract_tar_to_path`
- `muntarfs_extract_targz_to_path`
- `muntarfs-pack.sh`

