#!/bin/bash

# assume all binaries are in cjit-bin/release*/*
# from github action release assets

mkdir -p cjit-tutorial
cp -ra cjit-bin/release/*  cjit-tutorial
cp -ra examples            cjit-tutorial
git clone --depth 1 https://github.com/dyne/docs
cp -ra docs/src/cjit       cjit-tutorial/docs

zip a cjit-tutorial.zip    cjit-tutorial
