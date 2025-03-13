CC = gcc
CFLAGS = -I. -O2 -Wall -g
LDFLAGS = -lglfw -ldl -lGL -lGLU -lm -lSOIL -lSDL2 -lSDL2_mixer
OBJS = main.o Pacman.o

# Installation target for building and preparing the environment
install:
	sudo apt-get update
	sudo apt-get install -y build-essential libgl1-mesa-dev libglu1-mesa-dev libglfw3-dev libsoil-dev libsdl2-dev libsdl2-mixer-dev

all: main

main: $(OBJS)
	$(CC) -o Pacman $(OBJS) $(LDFLAGS)
	rm -rf *.o

main.o: main.c Pacman.h
	$(CC) $(CFLAGS) -c main.c

Pacman.o: Pacman.c Pacman.h
	$(CC) $(CFLAGS) -c Pacman.c

clean:
	rm -f Pacman *.o

run: Pacman
	./Pacman