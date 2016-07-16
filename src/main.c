#include "stdio.h"
#include "SDL.h"

#ifdef DEBUG
#define log(...) printf(__VA_ARGS__); printf("\n")
#else
#define log(...)
#endif

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

int main (int argc, char** argv ) {

    log( "Initializing SDL ..." );

    if (SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
        log( "Failed to initialize ..." );
        return -1;
    }

    log( "Creating window and renderer ..." ); 
    SDL_Window* window;
    SDL_Renderer* winrend;
    if ( SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN, &window, &winrend) < 0 ) {
        log( "Failed to create window and renderer ..." );
        return -1;
    }

    log( "Clearing white to the screen ...");
    SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );
    SDL_RenderClear( winrend );
    SDL_RenderPresent( winrend );

    log( "Waiting for quit event ..." );
    SDL_Event event;
    int done = 0;
    while ( !done ) {
        SDL_PollEvent( &event );

        if (event.type == SDL_QUIT) {
            done = 1;
        }
    }

    log( "Performing cleanup operations ..." );
    SDL_Quit( );
    return 0;
}
