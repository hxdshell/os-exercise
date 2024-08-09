CC=gcc
memory : 
	$(CC) -c test_alloc.c
	$(CC) -c alloc.c
	$(CC) alloc.o test_alloc.o -o alloct
	rm *.o

heap :
	$(CC) -c test_ealloc.c
	$(CC) -c ealloc.c
	$(CC) ealloc.o test_ealloc.o -o ealloc
	rm *.o