asan:
	gcc qoie.c -o bin/qoie -Wall -Wextra -g3 -fsanitize=address

debug:
	gcc qoie.c -o bin/qoie -Wall -Wextra -g3

release:
	gcc qoie.c -o bin/qoie -Wall -Wextra -O3
