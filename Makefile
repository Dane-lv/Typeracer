#── Compiler & flags
CC          := gcc
CFLAGS      := -Wall -Wextra -std=c11 -Ilib/include
SDL_CFLAGS  := $(shell pkg-config --cflags sdl3)
SDL_LDFLAGS := $(shell pkg-config --libs sdl3)

#── Directories & bins
BIN_DIR     := bin
CLIENT_BIN  := $(BIN_DIR)/client
SERVER_BIN  := $(BIN_DIR)/server

#── Source files
CLIENT_SRC  := client/src/main.c
SERVER_SRC  := server/main.c
LIB_SRCS    := $(wildcard lib/src/*.c)

#── Object files
CLIENT_OBJS := $(CLIENT_SRC:.c=.o) $(LIB_SRCS:.c=.o)
SERVER_OBJS := $(SERVER_SRC:.c=.o) $(LIB_SRCS:.c=.o)

#── Default target
.PHONY: all
all: $(CLIENT_BIN) $(SERVER_BIN)

#── Ensure bin directory exists
$(BIN_DIR):
	mkdir -p $@

#── Link client (with SDL)
$(CLIENT_BIN): $(CLIENT_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $^ -o $@ $(SDL_LDFLAGS)

#── Link server (also with SDL if it uses it)
$(SERVER_BIN): $(SERVER_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $^ -o $@ $(SDL_LDFLAGS)

#── Compile any .c --> .o
# This picks up client/src/*.c, server/*.c, and lib/src/*.c
%.o: %.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

#── Clean up
.PHONY: clean
clean:
	rm -rf $(CLIENT_OBJS) $(SERVER_OBJS) $(BIN_DIR)
