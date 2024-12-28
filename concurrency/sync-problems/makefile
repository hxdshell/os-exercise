CC=gcc

zemaphore: ../zemaphore/zemaphore.c
	$(CC) -c ../zemaphore/zemaphore.c

phil: zemaphore diningphil.c ../zemaphore/zemaphore.c ../zemaphore/zemaphore.h
	$(CC) -I ../zemaphore -c diningphil.c
	$(CC) diningphil.o zemaphore.o -o diningphil
	rm zemaphore.o
	rm diningphil.o

barbershop: zemaphore sleepingbarber.c ../zemaphore/zemaphore.c ../zemaphore/zemaphore.h
	$(CC) -I ../zemaphore -c sleepingbarber.c
	$(CC) sleepingbarber.o zemaphore.o -o barbershop
	rm zemaphore.o
	rm sleepingbarber.o
