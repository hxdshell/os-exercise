CC=gcc

dp: diningphil.c ../zemaphore/zemaphore.c ../zemaphore/zemaphore.h
	$(CC) -c ../zemaphore/zemaphore.c
	$(CC) -I ../zemaphore -c diningphil.c
	$(CC) diningphil.o zemaphore.o -o diningphil
	rm zemaphore.o
	rm diningphil.o
