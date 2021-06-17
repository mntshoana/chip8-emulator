flags = -Wpointer-arith -Wall -Wextra -pedantic -std=c++11 -g3

.PHONY: app
all: clean bin app test
bin:
	mkdir -p bin
db: bin src/main.cpp
	$(CXX) src/main.cpp			$(flags) -o bin/chip8-emulator.o
test: bin db test/tests.cpp
	$(CXX) test/tests.c 			$(flags) -o bin/test.o
clean:
	rm -dfr bin
