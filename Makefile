CC = gcc
CFLAGS = -I. -O2 -Wall -g
LDFLAGS = -lglfw -ldl -lGL -lGLU -lm -lSOIL
OBJS = main.o Pacman.o

all: main

main: $(OBJS)
	$(CC) -o main $(OBJS) $(LDFLAGS)

main.o: main.c Pacman.h
	$(CC) $(CFLAGS) -c main.c

Pacman.o: Pacman.c Pacman.h
	$(CC) $(CFLAGS) -c Pacman.c

clean:
	rm -f main *.o

run: main
	./main
