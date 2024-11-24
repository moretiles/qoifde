CFLAGS=-Wall -Wextra
DEBUG=-g3
SAN=-fsanitize=address -fsanitize=undefined
OPTIMIZE=-O3

debug: example.c qoi.h queue.h rgba.h tags 
	cc example.c -o bin/qoi ${CFLAGS} ${DEBUG} ${SAN}

gdb: example.c qoi.h queue.h rgba.h tags 
	cc example.c -o bin/qoi ${CFLAGS} ${DEBUG}

release: example.c qoi.h queue.h rgba.h tags 
	cc example.c -o bin/qoi ${CFLAGS} ${OPTIMIZE}

# There might be a better way to do this
tags: *.c *.h
	ctags -R || true

clean:
	rm -f tags bin/qoi obj/rgba.o obj/queue.o obj/qoi.o
