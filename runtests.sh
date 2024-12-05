#!/bin/bash

set -mex

if [[ -z "${2}" ]]; then
    echo "Provide make option" 1>&2
    exit 1
fi

make clean
make "${2}"
case "${1}" in
    e*)
        time ./bin/qoifde || printf "exited with abnormal status code %s\n" $?
        #cmp assets/dog1.qoi out/dog1.qoi
        #cmp assets/dog2.qoi out/dog2.qoi
        #cmp assets/dog3.qoi out/dog3.qoi
        #cmp assets/dog4.qoi out/dog4.qoi
        #cmp assets/dog5.qoi out/dog5.qoi
        #cmp assets/q1k3.qoi out/q1k3.qoi
        #cmp assets/test.qoi out/test.qoi
        #cmp assets/wall.qoi out/wall.qoi
        # look to see if all the images seem right
        sxiv out
        ;;
    d*)
        time ./bin/qoifde || printf "exited with abnormal status code %s\n" $?
        cmp assets/dog1.rgb out/dog1.rgb || true
        cmp assets/dog2.rgb out/dog2.rgb || true
        cmp assets/dog3.rgb out/dog3.rgb || true
        cmp assets/dog4.rgb out/dog4.rgb || true
        cmp assets/dog5.rgba out/dog5.rgba || true
        cmp assets/q1k3.rgba out/q1k3.rgba || true
        cmp assets/test.rgba out/test.rgba || true
        cmp assets/wall.rgba out/wall.rgba || true
        cmp assets/90s.rgba out/90s.rgba || true
        cmp assets/oldtime.rgb out/oldtime.rgb || true
        cmp assets/valley.rgb out/valley.rgb || true
        cmp assets/windows11.rgb out/windows11.rgb || true
        ;;
    *)
        echo "Don't even know that" 1>&2
        exit 1
        ;;
esac
