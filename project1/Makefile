main: main.o
	gcc -o main main.o
main.o: main.c my_malloc.c
	gcc -Wall -Werror -pedantic -std=gnu99 -c main.c my_malloc.c
