CC=gcc
memory : 
	$(CC) -c test_alloc.c
	$(CC) -c alloc.c
	$(CC) alloc.o test_alloc.o -o alloct
	rm *.o