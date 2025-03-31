# compilador
CC = gcc

BUILD_DIR = build

# archivitos
SRC_DIR = src
SRC = $(addprefix $(SRC_DIR)/, main.c)

# ejecutable principal
BIN = $(BUILD_DIR)/bin

FLAGS_LINKING = -lSDL3 -lSDL3_image -lm
FLAGS_COMMON  = -Wall -Wextra -Wpedantic -std=c23 
FLAGS_RELEASE = -O3
FLAGS_TESTING = -fsanitize=address,undefined -g

release: FLAGS = $(FLAGS_COMMON) $(FLAGS_RELEASE) $(FLAGS_LINKING)
release: $(BIN)

testing: FLAGS = $(FLAGS_COMMON) $(FLAGS_TESTING)
testing: $(BIN)

$(BIN): $(SRC)
	$(CC) $^ -o $@ $(FLAGS)

clean:
	-rm $(BIN)

test: $(BIN)
	valgrind --leak-check=full ./$(BIN)

run:
	./$(BIN)

run-release:
	$(MAKE) clean
	$(MAKE) release
	$(MAKE) run

run-sanitizer:
	$(MAKE) clean
	$(MAKE) testing
	$(MAKE) run
