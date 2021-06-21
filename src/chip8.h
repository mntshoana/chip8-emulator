#ifndef CHIP8_HEADER
#define CHIP8_HEADER

#include <cstdint>
#include <fstream>
#include <chrono>
#include <random>

#include "chip8-Constants.h"

const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

class Chip8 {
    // REFERENCE at: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
    uint8_t  registers[16]{}; // 16 registers
    uint8_t  memory[4096]{}; // 4K byte of memory
    uint16_t index{}; // 16 bit index register
    uint16_t pc{}; // 16 bit program counter
    uint16_t stack[16]{}; // 16 level stack
    uint8_t  sp{}; // 8 bit stack pointer
    uint8_t  delayTimer{}; // 8 bit delay timer
    uint8_t  soundTimer{}; // 8 bit sound timer
    
    uint16_t opcode; // for holding any of the 34 instructions
    

    std::default_random_engine ranomdGenerator;
    std::uniform_int_distribution<uint8_t> randDistribByte;
public:
    Chip8(); // Constructor
    
    void LoadROM(char const* filename);
    // Emulates the Fetch, Decode, Execute clock cycle of the Chip8 CPU
    void cycle();
    
    uint32_t displayMemory[64 * 32]{}; // 64x32 Monochrome Display Memory
    uint8_t  keypad[16]{}; // 16 input keys
    /*
     Keypad       Keyboard
     +-+-+-+-+    +-+-+-+-+
     |1|2|3|C|    |1|2|3|4|
     +-+-+-+-+    +-+-+-+-+
     |4|5|6|D|    |Q|W|E|R|
     +-+-+-+-+ => +-+-+-+-+
     |7|8|9|E|    |A|S|D|F|
     +-+-+-+-+    +-+-+-+-+
     |A|0|B|F|    |Z|X|C|V|
     +-+-+-+-+    +-+-+-+-+
     */
private:
// Functions to map to opcode
    // Clear the display
    void OP_00E0_CLS();
    // Returns from a subroutine
    void OP_00EE_RET();
    // Jumps to an address location
    void OP_1nnn_JP(); // note, instruction looks like: JP addr
    // Calls a subroutine at loacation
    void OP_2nnn_CALL();
    // Skips next instruction if Vx = kk. note, instruction looks like: SE Vx, byte
    void OP_3xkk_SE();
    // Skips next instruction if Vx != kk. note, instruction looks like: SNE Vx, byte
    void OP_4xkk_SNE();
    // Skips next instruction if Vx = Vy. note, instruction looks like: SE Vx, Vy
    void OP_5xy0_SE();
    // Sets Vx.
    void OP_6xkk_LD(); // note, instruction looks like LD Vx, byte
    void OP_8xy0_LD(); // note, instruction looks like LD Vx, Vy
    // Adds a value to register's value
    void OP_7xkk_ADD(); // note, instruction looks like ADD Vx, byte
    void OP_8xy4_ADD(); // note, instruction looks like ADD Vx, Vy
    // Sets Vx with result of OR operation: OR Vx, Vy
    void OP_8xy1_OR();
    // Sets Vx with result of AND operation: AND Vx, Vy
    void OP_8xy2_AND();
    // Sets Vx with result of XOR operation: XOR Vx, Vy
    void OP_8xy3_XOR();
    // Subtracts a value from a register's value
    void OP_8xy5_SUB(); // note, instruction looks like SUB Vx, Vy
    // shift bits to the right, by 1
    void OP_8xy6_SHR();
    // subtracts register value from another register value
    void OP_8xy7_SUBN(); // note, instruction looks like: SUBN Vx, Vy; sets Vx = Vy - Vx
    // shifts bits to the left, by 1
    void OP_8xyE_SHL();
    // Skips next instruction if Vx != Vy
    void OP_9xy0_SNE(); // note, instruction looks like: SNE Vx, Vy
    // Sets the index register to a given value
    void OP_Annn_LD(); // note, instruction looks like: LD I, addr
    // Jumps to the addr of V0 + nnn.
    void OP_Bnnn_JP(); // note, instruction looks like: JP V0, addr
    // Set Vx to: (random byte) AND kk.
    void OP_Cxkk_RND(); // note, instruction looks like: RND Vx, byte
    // Displays n-byte sprite from I at (Vx, Vy), and sets VF to express a collision.
    void OP_Dxyn_DRW(); // note, instruction looks like: DRW Vx, Vy, nibble
    // Skips the next instruction if user presses the key with the value of Vx
    void OP_Ex9E_SKP(); // note, instruction looks like: SKP Vx
    // Skips the next instruction if user does not press the key with the value of Vx
    void OP_ExA1_SKNP(); // note, instruction looks like: SKNP Vx
    // Sets Vx with a delay timer value
    void OP_Fx07_LD(); // note, instruction looks like: LD Vx, DT
    // Waits for a key press, then stores the value in Vx
    void OP_Fx0A_LD(); // note, instruction looks like: LD Vx, K
    // Sets the Delay timer to the value in Vx
    void OP_Fx15_LD(); // note, instruction looks like: LD DT, Vx
    // Sets the Sound timer to the value in Vx
    void OP_Fx18_LD(); // note, instruction looks like: LD ST, Vx
    // Adds the Index register with the value of Vx
    void OP_Fx1E_ADD(); // note, instruction looks like: ADD I, Vx
    // Set the index register with the address of the sprite representing a digit in Vx.
    void OP_Fx29_LD(); // note, instruction looks like: LD F, Vx
    // Stores the Binary Coded Decimal (BCD) of Vx in locations I, I+1, and I+2.
    void OP_Fx33_LD(); // note, instruction looks like: LD B, Vx
    // Stores registers V0 through Vx in memory, starting from location I
    void OP_Fx55_LD(); // note, instruction looks like: LD [I], Vx
    // Loads registers V0 through Vx from memory, starting from location I
    void OP_Fx65_LD(); // note, instruction looks like: LD Vx, [I]
    
    ///
    //  Mappings opcode to opcode functions
    //
    
    // Sets up the Pointer Table
    // This array is used to index the mapped opcode functions using the opcode itself
    void setUpPointerTable();
    // Helpers for setUpPointerTable
    void Table0();
    void Table8();
    void TableE();
    void TableF();
    void NULL_OP_DO_NOTHING();
    
    typedef void (Chip8::*opcodeTableFnPtr)();
    // Create table arrays and initialize them to point to default function with an empty body
    opcodeTableFnPtr table [0xF + 1] { &Chip8::NULL_OP_DO_NOTHING }; // main table pointer array
    opcodeTableFnPtr table0[0xE + 1]{ &Chip8::NULL_OP_DO_NOTHING }; // nested table pointer array
    opcodeTableFnPtr table8[0xE + 1]{ &Chip8::NULL_OP_DO_NOTHING }; // nested table pointer array
    opcodeTableFnPtr tableE[0xE + 1]{ &Chip8::NULL_OP_DO_NOTHING }; // nested table pointer array
    opcodeTableFnPtr tableF[0x65 + 1]{ &Chip8::NULL_OP_DO_NOTHING }; // nested table pointer array
    
};

#endif
