CC = gcc
CFLAGS = -I. -I/usr/include -L/usr/lib -lglfw -ldl -lGL -lm
SRC = main.c glad.c
OBJ = $(SRC:.c=.o)
EXEC = main

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
