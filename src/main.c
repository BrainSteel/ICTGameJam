#include "stdlib.h"

#include "SDL.h"

#include "Common.h"
#include "GameState.h"

#define SCREEN_WIDTH (1920/2)
#define SCREEN_HEIGHT (1080/2)

int main (int argc, char** argv ) {

    gamelog( "Initializing SDL ..." );

    if (SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
        gamelog( "Failed to initialize ..." );
        return -1;
    }

    gamelog( "Creating window and renderer ..." );
    SDL_Window* window;
    SDL_Renderer* winrend;
    if ( SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &winrend) < 0 ) {
        gamelog( "Failed to create window and renderer ..." );
        return -1;
    }

    gamelog( "Running game ..." );
    Run( window, winrend );

    gamelog( "Destroying window and renderer ...");
    SDL_DestroyWindow( window );
    SDL_DestroyRenderer( winrend );

    gamelog( "Quitting SDL ..." );
    SDL_Quit( );
    return 0;
}

int Run( SDL_Window* window, SDL_Renderer* winrend ) {

    // Get window width and height
    int screenwidth, screenheight;
    SDL_GetWindowSize( window, &screenwidth, &screenheight );

    Circle circ;
    circ.pos.x = 600;
    circ.pos.y = 200;
    circ.rad = 20;
    circ.vel.x = 50;
    circ.vel.y = 95;
    circ.acc.x = -21;
    circ.acc.y = 21;

    Circle circ2;
    circ2.pos.x = 800;
    circ2.pos.y = 500;
    circ2.vel.x = -84;
    circ2.vel.y = -60;
    circ2.acc.x = -18;
    circ2.acc.y = 6;
    circ2.rad = 20;

    SDL_SetRenderDrawColor( winrend, 0, 0, 0, SDL_ALPHA_OPAQUE );

    // Initialize the game state
    GameState* game = GME_InitializeDefault( );

    gamelog( "Waiting for quit event ..." );
    while ( 1 ) {
        CaptureInput( game );
        if ( game->quit ) {
            break;
        }
    }

    return 0;
}

void CaptureInput( GameState* state ) {

    SDL_Event event;
    while ( SDL_PollEvent(&event) ) {
        if ( event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) ) {
            state->quit = 1;
            return;
        }
    }

    state->player.input.keyboard = SDL_GetKeyboardState( &state->player.input.numkeys );
    int x, y;
    SDL_GetMouseState( &x, &y );
    state->player.input.mouse.x = x;
    state->player.input.mouse.y = y;
}
