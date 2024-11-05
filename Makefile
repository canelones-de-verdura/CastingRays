# compilador
CC = gcc

BUILD_DIR = build

# archivitos .o
OBJS = $(addprefix $(BUILD_DIR)/, main.o img.o render.o player.o)

# ejecutable principal
BIN = $(BUILD_DIR)/bin

FLAGS_COMMON  = -Wall -Wextra
FLAGS_RELEASE = -O3
FLAGS_TESTING = -fsanitize=address,undefined -g

release: FLAGS = $(FLAGS_COMMON) $(FLAGS_RELEASE)
release: $(BIN)

testing: FLAGS = $(FLAGS_COMMON) $(FLAGS_TESTING)
testing: $(BIN)

$(BIN): $(OBJS)
	$(CC) $^ -o $@ -lm `sdl2-config --cflags --libs` $(FLAGS)

$(BUILD_DIR)/main.o: src/main.c
	gcc -c $^ -o $@	$(FLAGS)


$(BUILD_DIR)/img.o: src/img.c
	gcc -c $^ -o $@ $(FLAGS)


$(BUILD_DIR)/render.o: src/render.c
	gcc -c $^ -o $@ $(FLAGS)


$(BUILD_DIR)/player.o: src/player.c
	gcc -c $^ -o $@ $(FLAGS)


clean:
	rm $(BIN)
	rm $(OBJS)

test: $(BIN)
	valgrind --leak-check=full ./$(BIN)
