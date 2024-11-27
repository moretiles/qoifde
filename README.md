# Quite Okay Image Format Decoder / Encoder

* Can encode rgb and rgba files into qoi images.
* Can decode qoi images into rgb or rgba files.
* All encoding/decoding is buffered. A six gigabyte image won't kill your RAM.

---

## How to use
* Read example.c to see how to encode or decode.
* Supply images, compile, and run the resulting binary to encode/decode them.

## Important Files
* example.c contains an example of how to use this project.
* Makefile contains the build.
* runtests.sh contains different (extremely informal) tests.
* LICENSE contains the Apache-2.0 license which this project uses.

## Header (Source) Files
* qoifde.h contains the encodeQOI and decodeQOI functions. Other functions may be changed at any time.
* queue.h contains an internal data structure used to store bytes using an array with a queue interface.
* rgba.h contains an internal data structure that holds a red, green, blue, and alpha channel byte.

## Created Directories
* assets contains input image files.
* obj contains object files (currently unused).
* bin contains executables output by the makefile.
* out contains output image files.
