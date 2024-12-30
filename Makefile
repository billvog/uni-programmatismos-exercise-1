CC = gcc
CFLAGS = -Wall -Wextra

SRCS = src/main.c src/catalog.c src/customer.c
OBJS = $(patsubst src/%.c,bin/%.o,$(SRCS))
TARGET = bin/main

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

bin/%.o: src/%.c bin
	$(CC) $(CFLAGS) -c $< -o $@

bin:
	mkdir bin

clean:
	rm -rf bin