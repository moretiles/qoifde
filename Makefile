debug: qoie.c queue.h tags 
	gcc qoie.c -o bin/qoie -Wall -Wextra -g3 -fsanitize=address -fsanitize=undefined

gdb: qoie.c queue.h tags 
	gcc qoie.c -o bin/qoie -Wall -Wextra -g3

release: qoie.c queue.h tags 
	gcc qoie.c -o bin/qoie -Wall -Wextra -O3

tags: *.c *.h
	ctags -R
