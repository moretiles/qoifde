#!/bin/bash

set -e

make all
./bin/qoie
./bin/qoitestrgba
#./bin/qoitestrgba
cmp out/test.qoi out/mine.qoi || true
echo test.qoi
xxd out/test.qoi | head -n 5
echo mine.qoi
xxd out/mine.qoi | head -n 5
