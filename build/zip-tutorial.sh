#!/bin/bash

# assume all binaries are in cjit-bin/release*/*
# from github action release assets

pwd
ls -l
mkdir -p cjit-tutorial
bins=`ls cjit-bin`
for i in ${bins}; do
	mv cjit-bin/${i} cjit-tutorial/`echo ${i} | sed 's/release/cjit/'`
done
cp -ra examples            cjit-tutorial
#git clone --depth 1 https://github.com/dyne/docs dyne-docs
#cp -ra dyne-docs/src/cjit       cjit-tutorial/docs

zip -r cjit-tutorial.zip    cjit-tutorial
