CFLAGS=-Wall -Wextra
DEBUG=-g3
SAN=-fsanitize=address -fsanitize=undefined
OPTIMIZE=-O3 -fno-strict-aliasing -flto

OBJS = obj/rgba.o obj/queue.o obj/qoi.o

obj/%.o : %.c %.h
	cc -c -o $@ $< ${CFLAGS}

obj/rgba.o : rgba.c rgba.h
obj/queue.o : queue.c queue.h

obj/qoi.o : qoi.c qoi.h obj/rgba.o obj/queue.o
	cc -c -o $@ $< ${CFLAGS}

debug: example.c ${OBJS} tags 
	cc example.c ${OBJS} -o bin/qoi ${CFLAGS} ${LINKFLAGS} ${DEBUG} ${SAN}

gdb: example.c ${OBJS} tags 
	cc example.c ${OBJS} -o bin/qoi ${CFLAGS} ${LINKFLAGS} ${DEBUG}

release: example.c ${OBJS} tags 
	cc example.c ${OBJS} -o bin/qoi ${CFLAGS} ${LINKFLAGS} ${OPTIMIZE}

# There might be a better way to do this
tags: *.c *.h
	ctags -R || true

clean:
	rm -f tags bin/qoi obj/rgba.o obj/queue.o obj/qoi.o
