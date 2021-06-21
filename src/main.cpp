#include "chip8.h"
#include "engine.h"

int main (int argc, char* argv[]){
    Chip8 cpu;
    Engine engine("CHIP-8 Emulator",
                    VIDEO_WIDTH, VIDEO_HEIGHT /*window*/,
                    VIDEO_WIDTH, VIDEO_HEIGHT /*texture*/);
    
   
    while (engine.getQuitFlag() != true){
        cpu.cycle();
        engine.processInput(cpu.keypad);
        engine.update(cpu.displayMemory, sizeof(cpu.displayMemory[0]) * VIDEO_WIDTH);
    }
}


