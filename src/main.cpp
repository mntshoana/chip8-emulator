#include "chip8.h"
#include "engine.h"

#include <thread>

int main (int argc, char* argv[]){
    // Scale video ratio. CHIP-8 is very small (64x32)
    int videoScaler = 10; // for now
    // Slow down the clock rate
    int delay = 3; // for now
    // Rom
    const char* path = "roms/tetris.ch8";

    Engine engine("CHIP-8 Emulator",
                    VIDEO_WIDTH * videoScaler,
                    VIDEO_HEIGHT * videoScaler, /*window*/
                    VIDEO_WIDTH, VIDEO_HEIGHT /*texture*/);
    
    
    Chip8 device(path);
    
    int scanLineInBytes = sizeof(device.displayMemory[0]) * VIDEO_WIDTH;
    
    typedef std::chrono::high_resolution_clock clk;
    auto lastTime = clk::now();
    while (engine.getQuitFlag() != true){
        engine.processInput(device.keypad);
        auto current = clk::now();
        float elapsed = std::chrono::duration<float, std::chrono::milliseconds::period>(current - lastTime).count();
        
        if (elapsed > delay){
            lastTime = current;
            device.cycle();
            engine.update(device.displayMemory, scanLineInBytes);
            
        }
        else
            std::this_thread::sleep_for (std::chrono::nanoseconds(20));
        
    }
    return 0;
}


