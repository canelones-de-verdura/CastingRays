# compilador
CC = gcc

BUILD_DIR = build

# archivitos
SRC_DIR = src
SRC = $(addprefix $(SRC_DIR)/, main.c)

# ejecutable principal
BIN = $(BUILD_DIR)/bin

FLAGS_COMMON  = -Wall -Wextra -Wpedantic -std=c23 -lSDL3 -lm
FLAGS_RELEASE = -O3
FLAGS_TESTING = -fsanitize=address,undefined -g

release: FLAGS = $(FLAGS_COMMON) $(FLAGS_RELEASE)
release: $(BIN)

testing: FLAGS = $(FLAGS_COMMON) $(FLAGS_TESTING)
testing: $(BIN)

$(BIN): $(SRC)
	$(CC) $^ -o $@ $(FLAGS)

clean:
	rm $(BIN)

test: $(BIN)
	valgrind --leak-check=full ./$(BIN)
