#include <SDL2/SDL.h>

class Engine { // using the SDL framework, this class is responsible for the graphics renderer, setting up the window and handling the input events
    SDL_Window* window{};
    SDL_Renderer* renderer{};
    SDL_Texture* texture{};
    
    bool quit_flag;
public:
    Engine(char const* title,
             int windowWidth, int windowHeight,
             int textureWidth, int textureHeight);
    ~Engine();

    // Update window
    void update(void const* buffer, int pitch);
    // key input handler
    void processInput(uint8_t* keys);
    bool getQuitFlag();
};
