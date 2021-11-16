CC = g++
CFLAGS = -Wall -g -std=c++11

main: main.o Minefield.o MineSolver.o
	$(CC) $(CFLAGS) -o main main.o Minefield.o MineSolver.o

fast: main.o Minefield.o MineSolver.o
	$(CC) -std=c++11 -O3 -o main main.o Minefield.o MineSolver.o

main.o: main.cpp Minefield.h MineSolver.h
	$(CC) $(CFLAGS) -c main.cpp

Minefield.o: Minefield.cpp Minefield.h
	$(CC) $(CFLAGS) -c Minefield.cpp

MineSolver.o: MineSolver.cpp MineSolver.h
	$(CC) $(CFLAGS) -c MineSolver.cpp

dnd: Minefield.o dnd.o
	$(CC) -std=c++11 -o puzzle Minefield.o dnd.o

dnd.o:
	$(CC) -std=c++11 -c dnd.cpp

clean:
	rm *.o main
