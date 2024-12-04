CFLAGS=-Wall -Wextra
DEBUG=-g3
SAN=-fsanitize=address -fsanitize=undefined
ANALYZE_GCC=-fanalyzer
ANALYZE_CLANG=-analyze-headers
OPTIMIZE=-O3

DIRS=assets obj bin out

analyze: example.c qoifde.h queue.h rgba.h tags ${DIRS}
	cc example.c -o /tmp/qoifde.debug ${CFLAGS} ${DEBUG} ${ANALYZE_GCC}
	rm /tmp/qoifde.debug
	scan-build ${ANALYZE_CLANG} -o /tmp/qoifde.debug make debug

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
