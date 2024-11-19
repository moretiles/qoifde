#!/bin/bash

set -e

if [[ -z "${1}" ]]; then
    echo "Provide make option" 1>&2
fi

make "${1}"
echo mine.qoi
time ./bin/qoie
echo test.qoi
#time ./bin/qoitestrgb
time ./bin/qoitestrgba
cmp out/test.qoi out/mine.qoi || true
echo mine.qoi
xxd out/mine.qoi | head -n 5
echo test.qoi
xxd out/test.qoi | head -n 5
