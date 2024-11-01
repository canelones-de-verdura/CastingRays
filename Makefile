CC = gcc
CFLAGS = -Wall -Wextra -std=c11

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

TARGET = main

all: $(TARGET)
	rm -f $(OBJ)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -lm

clean:
	rm -f $(OBJ) $(TARGET)
