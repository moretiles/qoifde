#!/bin/bash

set -mex

if [[ -z "${1}" ]]; then
    echo "Provide make option" 1>&2
fi

make "${1}"
time ./bin/qoie
#time ./bin/qoitestrgb
#time ./bin/qoitestrgba
time ./bin/qoitestq1k3rgba
#time ./bin/qoitestwallrgba
cmp out/test.qoi out/mine.qoi || true
xxd out/mine.qoi | head -n 5
xxd out/test.qoi | head -n 5
