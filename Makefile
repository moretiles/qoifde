debug:
	gcc qoie.c -o bin/qoie -Wall -Wextra -g3 -fsanitize=address -fsanitize=undefined

release:
	gcc qoie.c -o bin/qoie -Wall -Wextra -O3
