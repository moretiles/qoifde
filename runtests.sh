#!/bin/bash

set -mex

if [[ -z "${2}" ]]; then
    echo "Provide make option" 1>&2
    exit 1
fi

make "${2}"
case "${1}" in
    e*)
        time ./bin/qoifde
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
        time ./bin/qoifde
        cmp assets/dog1.rgb out/dog1.rgb
        cmp assets/dog2.rgb out/dog2.rgb
        cmp assets/dog3.rgb out/dog3.rgb
        cmp assets/dog4.rgb out/dog4.rgb
        cmp assets/dog5.rgb out/dog5.rgb
        cmp assets/q1k3.rgb out/q1k3.rgb
        cmp assets/test.rgb out/test.rgb
        cmp assets/wall.rgba out/wall.rgba
        ;;
    *)
        echo "Don't even know that" 1>&2
        exit 1
        ;;
esac
