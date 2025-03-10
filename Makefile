CC = gcc
CFLAGS = -I. -O2 -Wall
LDFLAGS = -lglfw -ldl -lGL -lGLU -lm -lSOIL
OBJS = main.o glad.o Pacman.o enemy.o

all: main

main: $(OBJS)
	$(CC) -o main $(OBJS) $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

glad.o: glad.c
	$(CC) $(CFLAGS) -c glad.c

Pacman.o: Pacman.c
	$(CC) $(CFLAGS) -c Pacman.c

clean:
	rm -f main $(OBJS)

run: main
	./main
