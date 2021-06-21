flags = -Wpointer-arith -Wall -Wextra -pedantic -std=c++11 -g3

# Windows part

## Requires SDL2
ifeq ($(OS), Windows_NT) # FOR WINDOWS
    SDL_FLAG = -IC:/mingwdev/include/SDL2
    SDL_FLAG += -LC:/mingwdev/lib -lmingw32 -lSDL2main -lSDL2
else ifeq ($(shell uname -s), Darwin) # FOR MAC
    SDL_FLAG = -I/Library/Frameworks/SDL2.framework/Headers 
    SDL_FLAG += -F/Library/Frameworks/
    SDL_FLAG += -I/Library/Frameworks/SDL2_image.framework/Headers
    SDL_FLAG += -framework SDL2
endif
####
#Begin
####

.PHONY: app
all: clean bin app test
bin:
	mkdir -p bin
app: bin src/main.cpp src/chip8.cpp src/engine.cpp
	$(CXX) $(SDL_FLAG) src/main.cpp src/chip8.cpp src/engine.cpp       $(flags) -o bin/chip8-emulator.o
test: bin app test/tests.cpp
	$(CXX) test/tests.c 						$(flags) -o bin/test.o
clean:
	rm -dfr bin
