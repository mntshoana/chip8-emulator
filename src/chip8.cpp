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
    
    // prepare array of function pointers for the opcode. 
    setUpPointerTable();
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
void Chip8::OP_8xyE_SHL(){
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

// instruction: JP V0, addr
// Jumps to the addr of V0 + nnn.
void Chip8::OP_Bnnn_JP(){
    uint16_t address = opcode & 0x0FFFu;

    pc = registers[0] + address;
}

// instruction: RND Vx, byte
// Set Vx to: (random byte) AND kk.
void Chip8::OP_Cxkk_RND(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = randDistribByte(ranomdGenerator) & byte;
}

// instruction: DRW Vx, Vy, nibble
// Displays n-byte sprite from memory of index register at (Vx, Vy), and sets VF to express a collision.
void Chip8::OP_Dxyn_DRW(){
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

// Instruction: SKP Vx
// Skips the next instruction if the user presses a key with the value of Vx
// Note, pc already incremented, so to skip the next instruction, increment pc only once to skip
void Chip8::OP_Ex9E_SKP(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];

    if (keypad[key])
    {
        pc += 2; // skips over next instruction
    }
}

// Instruction: SKNP Vx
// Skips the next instruction if user does not press the key with the value of Vx
// Note, pc already incremented, so to skip the next instruction, increment pc only once to skip
void Chip8::OP_ExA1_SKNP(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

        uint8_t key = registers[Vx];

        if (!keypad[key])
        {
            pc += 2; // skips over next instruction
        }
}

// Instruction:  LD Vx, DT
// Sets Vx with a delay timer value
void Chip8::OP_Fx07_LD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] = delayTimer;
}

// Instruction: LD Vx, K
// Waits for a key press, then stores the value in Vx
// Note, pc is already incremented by here, this function may decrement pc to repeat this instruction
void Chip8::OP_Fx0A_LD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    if (keypad[0])
        registers[Vx] = 0;
    else if (keypad[1])
        registers[Vx] = 1;
    else if (keypad[2])
        registers[Vx] = 2;
    else if (keypad[3])
        registers[Vx] = 3;
    else if (keypad[4])
        registers[Vx] = 4;
    else if (keypad[5])
        registers[Vx] = 5;
    else if (keypad[6])
        registers[Vx] = 6;
    else if (keypad[7])
        registers[Vx] = 7;
    else if (keypad[8])
        registers[Vx] = 8;
    else if (keypad[9])
        registers[Vx] = 9;
    else if (keypad[10])
        registers[Vx] = 10;
    else if (keypad[11])
        registers[Vx] = 11;
    else if (keypad[12])
        registers[Vx] = 12;
    else if (keypad[13])
        registers[Vx] = 13;
    else if (keypad[14])
        registers[Vx] = 14;
    else if (keypad[15])
        registers[Vx] = 15;
    else
        pc -= 2; // waits whenever a keypad value is not detected (by running the same instruction repeatedly)
}

// Instruction: LD DT, Vx
// Sets the Delay timer to the value in Vx
void Chip8::OP_Fx15_LD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    delayTimer = registers[Vx];
}

// Instruction: LD ST, Vx
// Sets the Sound timer to the value in Vx
void Chip8::OP_Fx18_LD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    soundTimer = registers[Vx];
}

// Instruction: ADD I, Vx
// Adds the Index register value with the value of Vx
void Chip8::OP_Fx1E_ADD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    index += registers[Vx];
}

// Instruction: LD F, Vx
// Set the index register with the address of the sprite representing a digit in Vx.
void Chip8::OP_Fx29_LD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = registers[Vx];
    
    // Font characters are 5 bytes each
    index = FONTSET_START_ADDRESS + (5 * digit);
}
// Instruction: LD B, Vx
// Stores the Binary Coded Decimal (BCD) of Vx in locations I, I+1, and I+2.
void Chip8::OP_Fx33_LD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = registers[Vx];
    
    // 8 bit = maximum of 255
    // oneth digit
    memory[index + 2] = value % 10; // eg, 255 % 10 = 5
    value /= 10; // eg 255 / 10 = 25.5

    // Tenth digit
    memory[index + 1] = value % 10; // eg 25 % 10 = 5
    value /= 10; // eg 25 / 10 = 2.5

    // Hundredth digit
    memory[index] = value % 10; // eg 2 % 10 = 2
}

// Instruction: LD [I], Vx
// Stores registers V0 through Vx in memory, starting from location I
void Chip8::OP_Fx55_LD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; ++i)
    {
        memory[index + i] = registers[i];
    }
}

// Instruction: LD Vx, [I]
// Loads registers V0 through Vx from memory, starting from location I
void Chip8::OP_Fx65_LD(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; ++i)
    {
        registers[i] = memory[index + i];
    }
}

// Sets up the Pointer Table
// This array is used to index the mapped opcode functions using the opcode itself
void Chip8::setUpPointerTable(){
    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::OP_1nnn_JP;
    table[0x2] = &Chip8::OP_2nnn_CALL;
    table[0x3] = &Chip8::OP_3xkk_SE;
    table[0x4] = &Chip8::OP_4xkk_SNE;
    table[0x5] = &Chip8::OP_5xy0_SE;
    table[0x6] = &Chip8::OP_6xkk_LD;
    table[0x7] = &Chip8::OP_7xkk_ADD;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9xy0_SNE;
    table[0xA] = &Chip8::OP_Annn_LD;
    table[0xB] = &Chip8::OP_Bnnn_JP;
    table[0xC] = &Chip8::OP_Cxkk_RND;
    table[0xD] = &Chip8::OP_Dxyn_DRW;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    table0[0x0] = &Chip8::OP_00E0_CLS;
    table0[0xE] = &Chip8::OP_00EE_RET;

    table8[0x0] = &Chip8::OP_8xy0_LD;
    table8[0x1] = &Chip8::OP_8xy1_OR;
    table8[0x2] = &Chip8::OP_8xy2_AND;
    table8[0x3] = &Chip8::OP_8xy3_XOR;
    table8[0x4] = &Chip8::OP_8xy4_ADD;
    table8[0x5] = &Chip8::OP_8xy5_SUB;
    table8[0x6] = &Chip8::OP_8xy6_SHR;
    table8[0x7] = &Chip8::OP_8xy7_SUBN;
    table8[0xE] = &Chip8::OP_8xyE_SHL;

    tableE[0x1] = &Chip8::OP_ExA1_SKNP;
    tableE[0xE] = &Chip8::OP_Ex9E_SKP;

    tableF[0x07] = &Chip8::OP_Fx07_LD;
    tableF[0x0A] = &Chip8::OP_Fx0A_LD;
    tableF[0x15] = &Chip8::OP_Fx15_LD;
    tableF[0x18] = &Chip8::OP_Fx18_LD;
    tableF[0x1E] = &Chip8::OP_Fx1E_ADD;
    tableF[0x29] = &Chip8::OP_Fx29_LD;
    tableF[0x33] = &Chip8::OP_Fx33_LD;
    tableF[0x55] = &Chip8::OP_Fx55_LD;
    tableF[0x65] = &Chip8::OP_Fx65_LD;
}

void Chip8::Table0()
{
    uint16_t ref = opcode & 0x000Fu;
    (this->*table0[ref])();
}

void Chip8::Table8()
{
    uint16_t ref = opcode & 0x000Fu;
    (this->*table8[ref])();
}

void Chip8::TableE()
{
    uint16_t ref = opcode & 0x000Fu;
    (this->*tableE[ref])();
}

void Chip8::TableF()
{
    uint16_t ref = opcode & 0x00FFu;
    (this->*tableF[ref])();
}

void Chip8::NULL_OP_DO_NOTHING(){
    // Do nothing
}

void Chip8::cycle(){
    // Fetch instruction using pc counter and then increment program counter
    opcode = (memory[pc] << 8u) | memory[pc + 1];
    pc += 2;
    // Execute opcode using appropriate function from the opcode table pointer
    uint16_t LeftMostDigit = opcode & 0xF000u) >> 12u;
    (this->*table[LeftMostDigit])();
    
    if (delayTimer > 0)
        // Decrement if it's been set
        --delayTimer;

    if (soundTimer > 0)
        // Decrement if it's been set
        --soundTimer;
}
