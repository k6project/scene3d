#include <SDL/SDL.h>
#include <SDL/SDL_vulkan.h>

int SDL_main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow(
        "SDL Application Main Window", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        512 , 512, SDL_WINDOW_VULKAN);

    int keepRunning = 1;
    while (keepRunning)
    {
        SDL_Event event;
        while ((SDL_PollEvent(&event)))
        {
            if (event.type == SDL_QUIT)
            {
                keepRunning = 0;
                break;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
