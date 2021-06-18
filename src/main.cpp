#include <cstdint>
#include <fstream>
class Chip8
{
public:
    // 16 registers
    uint8_t registers[16]{};
    // 4K byte of memory
    uint8_t memory[4096]{};
    // 16 bit index register
    uint16_t index{};
    // 16 bit program counter
    uint16_t pc{};
    // 16 level stack
    uint16_t stack[16]{};
    // 8 bit stack pointer
    uint8_t sp{};
    // 8 bit delay timer
    uint8_t delayTimer{};
    // 8 bit sound timer
    uint8_t soundTimer{};
    // 16 input keys
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
    uint8_t keypad[16]{};
    // 64x32 Monochrome Display Memory
    uint32_t video[64 * 32]{};
    // for holding any of the
    // CHIP-8 34 instructions that we need to emulate
    uint16_t opcode;
    
    // Constructor
    Chip8();
    
    void LoadROM(char const* filename);
};

const unsigned int START_ADDRESS = 0x200;

Chip8::Chip8(){
    // Initialize the program counter
    pc = START_ADDRESS;
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
