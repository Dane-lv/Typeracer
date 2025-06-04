#── Compiler & flags
CC          := gcc
CFLAGS      := -Wall -Wextra -std=c11 -Ilib/include

#── SDL3 & Extensions (include paths and libs)
SDL_CFLAGS  := -I/opt/homebrew/include
SDL_LDFLAGS := -L/opt/homebrew/lib \
               -lSDL3 -lSDL3_ttf -lSDL3_image -lSDL3_mixer -lSDL3_net \
               -Wl,-rpath,/opt/homebrew/lib

#── Source files
SRC         := src/main.c
LIB_SRCS    := $(wildcard lib/src/*.c)

#── Object files
OBJS        := $(SRC:.c=.o) $(LIB_SRCS:.c=.o)

#── Output binary
BIN         := app

#── Default target
.PHONY: all
all: $(BIN)

#── Link the binary
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $^ -o $@ $(SDL_LDFLAGS)

#── Compile any .c → .o
%.o: %.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

#── Clean
.PHONY: clean
clean:
	rm -f $(OBJS) $(BIN)
