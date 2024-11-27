CFLAGS=-Wall -Wextra
DEBUG=-g3
SAN=-fsanitize=address -fsanitize=undefined
OPTIMIZE=-O3

debug: example.c qoifde.h queue.h rgba.h tags 
	cc example.c -o bin/qoifde ${CFLAGS} ${DEBUG} ${SAN}

gdb: example.c qoifde.h queue.h rgba.h tags 
	cc example.c -o bin/qoifde ${CFLAGS} ${DEBUG}

release: example.c qoifde.h queue.h rgba.h tags 
	cc example.c -o bin/qoifde ${CFLAGS} ${OPTIMIZE}

# There might be a better way to do this
tags: *.c *.h
	ctags -R || true

clean:
	rm -f tags bin/qoifde obj/* out/*
