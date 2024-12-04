# Quite Okay Image Format Decoder / Encoder
* Can decode qoi images as rgb or rgba files.
* Can encode rgb and rgba files as qoi images.
* All encoding/decoding is buffered.

---

## How to use
```c
#include "qoifde.h"

int error;
int ret1, ret2, ret3;

//decode (QOI to RGB/RGBA)
ret1 = decodeQOI("wall.qoi", "wall.rgb",  3);
ret2 = decodeQOI("wall.qoi", "wall.rgba", 4);

//encode (RGB/RGBA TO QOI)
int width = 480;
int height = 270;
char channels = 3;
char colorspace = 1;
ret3 = encodeQOI("wall.rgb", "wall.qoi", width, height, channels, colorspace);

error = ret1 | ret2 | ret3;
return error;
```

## Important functions
* decodeQOI accepts input filename (qoi file), output filename (rgb or rgba file), and output channels as arguments.
* encodeQOI accepts input filename (rgb or rgba file), output filename (qoi file), width, height, channels, and colorspace as arguments.
* All other functions are internal and may change.

## Important files
* qoifde.h needs to be included to decode or encode anything.
* example.c contains an example that I use for testing.
* Makefile contains a test build.
* LICENSE contains the Apache-2.0 license which this project uses.
