#main: main.o
#	gcc -o main main.o
#main.o:
#	gcc -Wall -Werror -pedantic -std=gnu99 -c main.c

libmymalloc.so: my_malloc.o
	gcc -shared -o libmymalloc.so my_malloc.o

my_malloc.o: my_malloc.c
	gcc -Wall -Werror -pedantic -std=gnu99 -fPIC -c my_malloc.c


#main: main.o
#	gcc -o main main.o
#main.o: main.c my_malloc.o
#	gcc -Wall -Werror -pedantic -std=gnu99 -c main.c my_malloc.o
#my_malloc.o: my_malloc.c
#	gcc -Wall -Werror -pedantic -std=gnu99 -c  my_malloc.c
