#!/bin/sh
set -e
echo
echo "Welcome to CJIT!"
echo "We'll be downloading our quick demo setup, please wait a bit,"
echo "then all will be found inside the 'cjit-demo' folder right here."
curl -sLo cjit https://github.com/dyne/cjit/releases/latest/download/cjit-$(uname)-$(uname -m)
chmod +x cjit
curl -sLo cjit-demo.tar.gz https://github.com/dyne/cjit/releases/latest/download/cjit-demo.tar.gz
./cjit --xtgz cjit-demo.tar.gz
cp ./cjit cjit-demo/
cd cjit-demo
echo "Ready to start! Follow the tutorial:"
echo " --> https://dyne.org/docs/cjit <--"
echo
exit 0
