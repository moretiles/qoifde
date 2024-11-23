debug: example.c qoie.h queue.h tags 
	cc example.c -o bin/qoie -Wall -Wextra -g3 -fsanitize=address -fsanitize=undefined

gdb: example.c qoie.h queue.h tags 
	cc example.c -o bin/qoie -Wall -Wextra -g3

release: example.c qoie.h queue.h tags 
	cc example.c -o bin/qoie -Wall -Wextra -O3

# There might be a better way to do this
tags: *.c *.h
	ctags -R || true
