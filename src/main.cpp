#include "chip8.h"
#include "engine.h"

#include <iostream>
#include <thread>

int main (int argc, char* argv[]){

    // Scale video ratio. CHIP-8 is very small (64x32)
    int videoScaler = 10; // for now
    // Slow down the clock rate
    int delay = 3; // for now
    // Rom
    char* path = "roms/tetris.ch8";
    
    if (argc == 2){ // prgName [Rom]
        path = argv[1];
    }
    else if (argc == 4){ // prgName [videoScaler] [delay] [Rom]
        videoScaler = std::stoi(argv[1]);
        delay = std::stoi(argv[2]);
        path = argv[4];
    }
    else {
        std::cerr << "NOTE!!!" << std::endl;
        std::cerr << " - Arguments where not properly provided. Using defaule of 10 3 \"rom/tetris.ch8\"" << std::endl;
    }
    
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


