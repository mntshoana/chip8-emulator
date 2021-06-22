# A simple project to mimic the components of a CHIP-8 in the form of an emulator.
This emulator is designed to fortify my understanding of how a CPU works. The excercise works through a dissassembler, instruction set and clock cycle emulation as well as generating images to the screen.  

To build this project, simply clone this project and then compile it as follows:
````
make all
````
To run the emulator
````
bin/chip8-emulator.o
````
This will run the emulator with the default rom of tetris.ch8, however, it accepts arguments as follows:
````
bin/chip8-emulator.o [video-Scaler-Int] [delay-Time-Int] [path to rom]
````
 - the first 2 are optional
 - if you provide an incorrect path, the emulator will crash. /*Todo*/
