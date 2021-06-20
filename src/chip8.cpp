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

// Sets the program counter to addr
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
void Chip8::OP_3xkk_SE(){
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
void Chip8::OP_4xkk_SNE(){
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
void Chip8::OP_5xy0_SE(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; // snuff out 8 lower order bits
    uint8_t Vy = (opcode & 0x00F0u) >> 4u; // snuff out 4 lower order bits

    if (registers[Vx] == registers[Vy])
    {
        pc += 2; // skips over next instruction
    }
}

// Instruction: LD Vx, byte
// Sets a register ( Vx)
void Chip8::OP_6xkk_LD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; // snuff out 8 lower order bits
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = byte;
}

// Adds
void Chip8::OP_7xkk_ADD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; // remove 8 lower order bits
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] += byte;
}

// Instruction: LD Vx, Vy
// Sets a register ( Vx) with contents of another register
void Chip8::OP_8xy0_LD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
}

// Instruction: OR Vx, Vy
void Chip8::OP_8xy1_OR(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] |= registers[Vy];
}

// Instruction: AND Vx, Vy
void Chip8::OP_8xy2_AND(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] &= registers[Vy];
}

// Instruction: XOR Vx, Vy
void Chip8::OP_8xy3_XOR(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] ^= registers[Vy];
}

// Adds
void Chip8::OP_8xy4_ADD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; // snuff out 8 lower order bits
    uint8_t Vy = (opcode & 0x00F0u) >> 4u; // snuff out 4 lower order bits

    uint16_t sum = registers[Vx] + registers[Vy];

    if (sum > 255U)
    { // Set overflow bit, VF
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[Vx] = sum & 0xFFu; // only store 8 bits
}

// Subtracts
void Chip8::OP_8xy5_SUB(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; // snuff out 8 lower order bits
    uint8_t Vy = (opcode & 0x00F0u) >> 4u; // snuff out 4 lower order bits

    if (registers[Vx] > registers[Vy])
    { // Set NOT borrow bit, VF
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[Vx] -= registers[Vy];
}

// Instruction: SHR Vx
// Shifts bits to the right by 1
void Chip8::OP_8xy6_SHR(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u; // snuff out 8 lower order bits
    
    // If most-significant bit is 1, then VF is set to 1
    registers[0xF] = (registers[Vx] & 0x1u); // Save LSB in VF

    registers[Vx] >>= 1;
}

// subtracts register value from another register value
// note, SUBN Vx, Vy sets Vx = Vy - Vx
void Chip8::OP_8xy7_SUBN()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vy] > registers[Vx])
    { //  set VF = NOT borrow.
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }
    // the result is stored in Vx.
    registers[Vx] = registers[Vy] - registers[Vx];
}

// shifts bits to the left, by 1
void OP_8xyE_SHL(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // Save MSB in VF
    // If most-significant bit is 1, then VF is set to 1
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
    
    registers[Vx] <<= 1;
}

// Instruction: SNE Vx, Vy
// Skips the following instruction on a condition that Vx != vy
// Note, pc already incremented, so to skip the next instruction, increment pc only once to skip
void Chip8::OP_9xy0_SNE()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] != registers[Vy])
    {
        pc += 2; // skips over next instruction
    }
}

// instruction: LD I, addr
// Sets a register the index register to a given address ( I)
void Chip8::OP_Annn_LD(){
    uint16_t address = opcode & 0x0FFFu;

    index = address;
}

// instruction: RND Vx, byte
// Set Vx to: (random byte) AND kk.
void Chip8::OP_Cxkk_RND(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = randByte(randGen) & byte;
}

// instruction: DRW Vx, Vy, nibble
// Displays n-byte sprite from memory of index register at (Vx, Vy), and sets VF to express a collision.
void OP_Dxyn_DRW(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = opcode & 0x000Fu; // n-bytes

    // Wraps around screen beyond boundaries
    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    // Reset VF in order to use it to express collisions
    registers[0xF] = 0;

    for (unsigned int row = 0; row < height; ++row)
    {
        // from memory of index register until n-bytse
        uint8_t spriteByte = memory[index + row];
        // There may be a screen pixel collision with what’s already being displayed
        for (unsigned int col = 0; col < 8; ++col)
        { // sprites are eight bits wide (represents 8 pixels of screen width)
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            // retrieve current pixel
            uint32_t* screenPixel = &displayMemory[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            // Sprite pixel is on
            if (spritePixel)
            {
                // Screen pixel is also on
                if (*screenPixel == 0xFFFFFFFF)
                {// equals collision
                    registers[0xF] = 1;
                }

                // Effectively XOR with the sprite pixel
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}
