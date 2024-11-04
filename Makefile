# compilador
CC = gcc

BUILD_DIR = build

# archivitos .o
OBJS = $(addprefix $(BUILD_DIR)/, main.o img.o render.o player.o)

# ejecutable principal
BIN = $(BUILD_DIR)/bin

$(BIN): $(OBJS)
	$(CC) $^ -o $@ -lm -O3 `sdl2-config --cflags --libs`

$(BUILD_DIR)/main.o: src/main.c
	gcc -c $^ -o $@	-O3

$(BUILD_DIR)/img.o: src/img.c
	gcc -c $^ -o $@ -O3

$(BUILD_DIR)/render.o: src/render.c
	gcc -c $^ -o $@ -O3

$(BUILD_DIR)/player.o: src/player.c
	gcc -c $^ -o $@ -O3

clean:
	rm $(BIN)
	rm $(OBJS)

test: $(BIN)
	# sacamos el valgrind
	valgrind --leak-check=full ./$(BIN)
