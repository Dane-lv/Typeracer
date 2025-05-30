CC = gcc
CFLAGS = -Wall -Wextra -std=c11 $(shell pkg-config --cflags sdl3)
LDFLAGS = $(shell pkg-config --libs sdl3)

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=src/%.o)
TARGET = game

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)
