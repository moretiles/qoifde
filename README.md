# Buffered QOI Encoder
* Converts a rgba pixels file to a qoi image.

---

# Files
* example.c contains an example of how to use the program.
* Makefile contains an example build.
* runtests.sh allows me to perform a byte comparison of the results of my program vs the reference implementation.
* qoie.h contains the encoding function and a number of internal functions.
* queue.h contains an internal queue data structure used to store bytes.
* tags is a ctags file (feel free to ignore it).

# Directories
* bin contains executables. All the qoitest* executables are the reference implementation.
* assets contains input image files.
* out contains output image files.
