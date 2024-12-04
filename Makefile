CFLAGS=-Wall -Wextra
DEBUG=-g3
SAN=-fsanitize=address -fsanitize=undefined
ANALYZE_GCC=-fanalyzer
ANALYZE_CLANG=-analyze-headers
OPTIMIZE=-O3

analyze: example.c qoifde.h tags
	rm -rf /tmp/qoifde.debug
	cc example.c -o /tmp/qoifde.debug ${CFLAGS} ${DEBUG} ${ANALYZE_GCC}
	rm -rf /tmp/qoifde.debug
	scan-build ${ANALYZE_CLANG} -o /tmp/qoifde.debug make debug
	rm -rf /tmp/qoifde.debug

debug: example.c qoifde.h tags
	cc example.c -o bin/qoifde ${CFLAGS} ${DEBUG} ${SAN}

gdb: example.c qoifde.h tags
	cc example.c -o bin/qoifde ${CFLAGS} ${DEBUG}

release: example.c qoifde.h tags
	cc example.c -o bin/qoifde ${CFLAGS} ${OPTIMIZE}

# There might be a better way to do this
tags: *.c *.h
	ctags -R || true

clean:
	rm -f tags README.pdf bin/qoifde obj/* out/*
