SRC = src/main.c

all: main

bin:
	mkdir bin

main: $(SRC) bin
	gcc -o bin/main $(SRC)

clean:
	rm -rf bin