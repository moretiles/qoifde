CFLAGS=-Wall -Wextra
DEBUG=-g3
SAN=-fsanitize=address -fsanitize=undefined
OPTIMIZE=-O3

DIRS=assets obj bin out

debug: example.c qoifde.h queue.h rgba.h tags ${DIRS}
	cc example.c -o bin/qoifde ${CFLAGS} ${DEBUG} ${SAN}

gdb: example.c qoifde.h queue.h rgba.h tags ${DIRS}
	cc example.c -o bin/qoifde ${CFLAGS} ${DEBUG}

release: example.c qoifde.h queue.h rgba.h tags ${DIRS}
	cc example.c -o bin/qoifde ${CFLAGS} ${OPTIMIZE}

# There might be a better way to do this
tags: *.c *.h
	ctags -R || true

assets:
	mkdir -p assets

obj:
	mkdir -p obj

bin:
	mkdir -p bin

out:
	mkdir -p out

clean:
	rm -f tags bin/qoifde obj/* out/*
