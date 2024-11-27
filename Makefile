CFLAGS=-Wall -Wextra
DEBUG=-g3
SAN=-fsanitize=address -fsanitize=undefined
OPTIMIZE=-O3

debug: example.c qoiconv.h queue.h rgba.h tags 
	cc example.c -o bin/qoiconv ${CFLAGS} ${DEBUG} ${SAN}

gdb: example.c qoiconv.h queue.h rgba.h tags 
	cc example.c -o bin/qoiconv ${CFLAGS} ${DEBUG}

release: example.c qoiconv.h queue.h rgba.h tags 
	cc example.c -o bin/qoiconv ${CFLAGS} ${OPTIMIZE}

# There might be a better way to do this
tags: *.c *.h
	ctags -R || true

clean:
	rm -f tags bin/qoiconv obj/rgba.o obj/queue.o obj/qoiconv.o
