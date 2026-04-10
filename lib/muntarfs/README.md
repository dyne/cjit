# muntarfs

`muntarfs` is the filesystem bundle component extracted from CJIT.

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

Non-goals:

- CJIT-specific runtime policy
- process management
- tempdir policy
- compiler integration
- general-purpose archive management beyond the bundle use-case

## Public Surface

- `muntarfs_extract_tar_to_path`
- `muntarfs_extract_targz_to_path`
- `muntarfs-pack.sh`

## Current Implementation Notes

This first extraction step keeps using the existing handwritten tar and gzip extraction primitives from CJIT.
The goal is to establish a reusable `lib/` surface first, then migrate ownership more deeply in later steps.
