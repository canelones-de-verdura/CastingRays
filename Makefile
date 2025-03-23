# compilador
CC = gcc

BUILD_DIR = build

# archivitos
SRC_DIR = src
SRC = $(addprefix $(SRC_DIR)/, main.c)

# ejecutable principal
BIN = $(BUILD_DIR)/bin

# asquerosidad para poder usar SDL3_image
# TODO sacar las flags a la mierda cuando la pueda instalar con dnf
FLAGS_LINKING = -lSDL3 -Wl,-rpath='./include/SDL3_image/' ./include/SDL3_image/libSDL3_image.so -lm
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
