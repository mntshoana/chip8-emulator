#include "engine.h"
Engine::Engine(char const* title,
                   int windowWidth, int windowHeight,
                   int textureWidth, int textureHeight){
    // Initialize SDL for using SDL function
    SDL_Init(SDL_INIT_VIDEO);
    // Window settings
    window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    // 2D rendering context using hardware acceleration
    // note, each driver (e.g. OpenGL, Direct3d, Software,…) is indexed in SDL 2.0 thus SDL uses -1 to pick one for us
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888 /*pixel format*/,
                            SDL_TEXTUREACCESS_STREAMING /*access modifier*/,
                            textureWidth, textureHeight);
    
    quit_flag = false;
}

Engine::~Engine() {
    // Clean up
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Engine::update(void const* buffer, int pitch) {
    // Updating the window
    // update given texture rectangle with new pixel data.
    SDL_UpdateTexture(texture, nullptr /*entire texture area*/, buffer, pitch /*nr of bytes per row in buffer*/);
    SDL_RenderClear(renderer); // ignores the viewport
    // Copy texture to the current rendering target
    SDL_RenderCopy(renderer, texture,
                   nullptr /*entire source (texture) area*/,
                   nullptr /*entire target area*/);
    SDL_RenderPresent(renderer);
}

void Engine::processInput(uint8_t* keys) {
    SDL_Event event; // receive input from the user

    while (SDL_PollEvent(&event)) { // read all from event queue
        switch (event.type) {
            case SDL_QUIT:
                quit_flag = true;
                break;
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit_flag = true;
                        break;
                    case SDLK_x:
                        keys[0] = 1;
                        break;
                    case SDLK_1:
                        keys[1] = 1;
                        break;
                    case SDLK_2:
                        keys[2] = 1;
                        break;
                    case SDLK_3:
                        keys[3] = 1;
                        break;
                    case SDLK_q:
                        keys[4] = 1;
                        break;
                    case SDLK_w:
                        keys[5] = 1;
                        break;
                    case SDLK_e:
                        keys[6] = 1;
                        break;
                    case SDLK_a:
                        keys[7] = 1;
                        break;
                    case SDLK_s:
                        keys[8] = 1;
                        break;
                    case SDLK_d:
                        keys[9] = 1;
                        break;
                    case SDLK_z:
                        keys[0xA] = 1;
                        break;
                    case SDLK_c:
                        keys[0xB] = 1;
                        break;
                    case SDLK_4:
                        keys[0xC] = 1;
                        break;
                    case SDLK_r:
                        keys[0xD] = 1;
                        break;
                    case SDLK_f:
                        keys[0xE] = 1;
                        break;
                    case SDLK_v:
                        keys[0xF] = 1;
                        break;
                    }
            } // ends nested switch
                break; // ends case SDL_KEYDOWN
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {// special key strokes
                    case SDLK_x:
                        keys[0] = 0;
                        break;
                    case SDLK_1:
                        keys[1] = 0;
                        break;
                    case SDLK_2:
                        keys[2] = 0;
                        break;
                    case SDLK_3:
                        keys[3] = 0;
                        break;
                    case SDLK_q:
                        keys[4] = 0;
                        break;
                    case SDLK_w:
                        keys[5] = 0;
                        break;
                    case SDLK_e:
                        keys[6] = 0;
                        break;
                    case SDLK_a:
                        keys[7] = 0;
                        break;
                    case SDLK_s:
                        keys[8] = 0;
                        break;
                    case SDLK_d:
                        keys[9] = 0;
                        break;
                    case SDLK_z:
                        keys[0xA] = 0;
                        break;
                    case SDLK_c:
                        keys[0xB] = 0;
                        break;
                    case SDLK_4:
                        keys[0xC] = 0;
                        break;
                    case SDLK_r:
                        keys[0xD] = 0;
                        break;
                    case SDLK_f:
                        keys[0xE] = 0;
                        break;

                    case SDLK_v:
                        keys[0xF] = 0;
                        break;
                } // end nested switch
                break; // ends case SDL_KEYDOWN
        } // ends outer switch body
    } // end while loop
}

bool Engine::getQuitFlag(){
    return quit_flag;
}
