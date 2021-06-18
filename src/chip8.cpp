#include "chip8.h"

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_START_ADDRESS = 0x50;

Chip8::Chip8() {
    // Initialize the program counter
    pc = START_ADDRESS;
    
    // Load fonts into memory
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
    
    // handle instruction which generates a random number into a register
    // normally achieved by, reading the value from a noisy disconnected pin or using a dedicated RNG chip
    ranomdGenerator = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
    randDistribByte = std::uniform_int_distribution<uint8_t>(0, 255U); // between 0 and 255
}

void Chip8::LoadROM(char const* filename){
    // File will carry stream of binary with instructions
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()){
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        // Return pointer to the beginning and fill the buffer
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        // NB! Memory from 0x000 to 0x1FF is reserved
        // Load ROM contents from 0x200
        for (long i = 0; i < size; ++i){
            memory[START_ADDRESS + i] = buffer[i];
        }

        // Cleanup
        delete[] buffer;
    }
    return;
}

// Sets the entire video buffer to zeroes
void Chip8::OP_00E0_CLS(){
    memset(displayMemory, 0, sizeof(displayMemory));
}

// Reloads the address of the instruction past the one that called the subroutine (which is at the top of the stack) back into the PC.
void Chip8::OP_00EE_RET(){
    --sp;
    pc = stack[sp];
}

// Sets the program counter to nnn.
// No stack interaction required for a jump
void Chip8::OP_1nnn_JP(){
    uint16_t address = opcode & 0x0FFFu;
    pc = address;
}

// Calls a subroutine and stores the current PC onto the top of the stack (current PC already holds the next instruction after this CALL)
void Chip8::OP_2nnn_CALL(){
    uint16_t address = opcode & 0x0FFFu;

    stack[sp] = pc;
    ++sp;
    pc = address;
}

// Instruction: SE Vx, byte
// Skips the following instruction on a condition that Vx = kk
// Note, pc already incremented, so to skip the next instruction, increment pc only once to skip
void OP_3xkk_SE(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; // snuff out 8 lower order bits
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] == byte)
    {
        pc += 2; // skips next instruction here because pc is already incremented
    }
}

// Instruction: SNE Vx, byte
// Skips the following instruction on a condition that Vx != kk
// Note, pc already incremented, so to skip the next instruction, increment pc only once to skip
void OP_4xkk_SNE(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; // remove 8 lower order bits
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] != byte)
    {
        pc += 2; // skips the next instruction
    }
}

// Instruction: SE Vx, Vy
// Skips the following instruction on a condition that Vx = vy
// Note, pc already incremented, so to skip the next instruction, increment pc only once to skip
void OP_3xkk_SE(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; // snuff out 8 lower order bits
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] == byte)
    {
        pc += 2; // skips next instruction
    }
}
void OP_5xy0_SE(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; // snuff out 8 lower order bits
    uint8_t Vy = (opcode & 0x00F0u) >> 4u; // snuff out 4 lower order bits

    if (registers[Vx] == registers[Vy])
    {
        pc += 2; // skips over next instruction
    }
}
