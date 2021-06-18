flags = -Wpointer-arith -Wall -Wextra -pedantic -std=c++11 -g3

.PHONY: app
all: clean bin app test
bin:
	mkdir -p bin
app: bin src/main.cpp src/chip8.cpp
	$(CXX) src/main.cpp src/chip8.cpp		$(flags) -o bin/chip8-emulator.o
test: bin app test/tests.cpp
	$(CXX) test/tests.c 			$(flags) -o bin/test.o
clean:
	rm -dfr bin
