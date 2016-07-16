#include "stdlib.h"

#include "SDL.h"

#include "Common.h"
#include "GameState.h"

#define SCREEN_WIDTH (1920)
#define SCREEN_HEIGHT (1080)
#define FRAMERATE 40

#define MAP_WIDTH SCREEN_WIDTH * 5
#define MAP_HEIGHT SCREEN_HEIGHT * 5

#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 50

void DrawMiniMap( SDL_Renderer* winrend, GameState* game );
void CreateWorld( SDL_Renderer* winrend, SDL_Texture* background, World* world, int width, int height);

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

    SDL_Surface* Background = SDL_LoadBMP("rsc/Background.bmp");
    if(!Background)
    {
        printf("%s", SDL_GetError());
        printf("ERROR-> Background NOT LOADED");
        return 1;
    }
    SDL_SetColorKey(Background, SDL_TRUE, SDL_MapRGB(Background->format, 0, 0, 0));


    SDL_Surface* Player = SDL_LoadBMP("rsc/Player.bmp");
    if(!Player)
    {
        printf("ERROR-> Player NOT LOADED");
        return 1;
    }
    SDL_SetColorKey(Player, SDL_TRUE, SDL_MapRGB(Player->format, 0, 0, 0));

    SDL_Texture* BackgroundTex = SDL_CreateTextureFromSurface(winrend, Background);
    SDL_Texture* PlayerTex = SDL_CreateTextureFromSurface(winrend, Player);

    SDL_FreeSurface(Background);
    SDL_FreeSurface(Player);

    // Initialize the game state
    GameState* game = GME_InitializeDefault( );
    game->player.Player_TEX = PlayerTex;

    game->viewableWorld.height = SCREEN_HEIGHT;
    game->viewableWorld.width = SCREEN_WIDTH;
    game->viewableWorld.centerY = MAP_HEIGHT / 2;
    game->viewableWorld.centerX = MAP_WIDTH / 2;

    gamelog( "Creating world ...");
    CreateWorld(winrend, BackgroundTex, &game->world, 5000, 5000);

    gamelog( "Running game ..." );
    Run( window, winrend, game );

    gamelog( "Destroying window and renderer ...");
    SDL_DestroyWindow( window );
    SDL_DestroyRenderer( winrend );

    gamelog( "Destroying textures ...");
    SDL_DestroyTexture(BackgroundTex);
    SDL_DestroyTexture(PlayerTex);

    gamelog( "Quitting SDL ..." );
    SDL_Quit( );
    return 0;
}

int Run( SDL_Window* window, SDL_Renderer* winrend, GameState* game ) {

    // Get window width and height
    int screenwidth, screenheight;
    SDL_GetWindowSize( window, &screenwidth, &screenheight );

    SDL_SetRenderDrawColor( winrend, 0, 0, 0, SDL_ALPHA_OPAQUE );

    gamelog( "Clearing white to the screen ...");
    SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );
    SDL_RenderClear( winrend );
    SDL_RenderPresent( winrend );

    game->player.entity.body.shape.pos.x = SCREEN_WIDTH / 2;
    game->player.entity.body.shape.pos.y = SCREEN_HEIGHT / 2;
    game->player.entity.body.shape.rad = 20;
    Component booster;
    booster.strength = 1.0;
    booster.mass = 1.7;
    booster.ability = Booster;
    booster.health = 10;
    booster.shape.rad = 10;
    booster.shape.pos.x = game->player.entity.body.shape.pos.x + 30;
    booster.shape.pos.y = game->player.entity.body.shape.pos.y;
    Attach( &game->player, booster );
    booster.shape.pos.x = game->player.entity.body.shape.pos.x - 30;
    Attach( &game->player, booster );
    booster.shape.pos.x = game->player.entity.body.shape.pos.x;
    booster.shape.pos.y = game->player.entity.body.shape.pos.y - 30;
    Attach( &game->player, booster );

    gamelog( "Waiting for quit event ..." );
    uint64_t starttime;
    while ( 1 ) {
        starttime = SDL_GetTicks( );

        CaptureInput( game );
        if ( game->quit ) {
            break;
        }

        // Reset player accelerations
        game->player.entity.body.shape.acc.x = 0.0f;
        game->player.entity.body.shape.acc.y = 0.0f;
        game->player.entity.angacc = 0.0f;

        PerformAction( &game->player, Booster );

        UpdatePlayer( &game->player, 1.0 );

        SDL_RenderClear( winrend );
        SDL_RenderCopy( winrend, game->world.background, NULL, NULL );
        DrawPlayer( winrend, &game->player, (Vector2){ 0.0f, 0.0f });
        SDL_RenderPresent( winrend );

        uint64_t endtime = SDL_GetTicks( );
        if ( endtime - starttime < 1000 / FRAMERATE) {
            SDL_Delay(( 1000 / FRAMERATE ) - ( SDL_GetTicks() - starttime ));
        }
        else {
            gamelog( "Frame took longer than expected: %llu ms", endtime - starttime );
        }
    }

    return 0;
}

void DrawMiniMap( SDL_Renderer *winrend, GameState* game )
{
    SDL_Rect miniMap;
    miniMap.h = (game->viewableWorld.height / 5);
    miniMap.w = (game->viewableWorld.width / 5);
    miniMap.x = 1145;
    miniMap.y = 5;

    SDL_RenderCopy(winrend, game->world.background, NULL, &miniMap);

    SDL_Rect miniMapBoarder;
    miniMapBoarder.h = (game->viewableWorld.height / 5);
    miniMapBoarder.w = (game->viewableWorld.width / 5);
    miniMapBoarder.x = 1145;
    miniMapBoarder.y = 5;

    SDL_RenderDrawRect(winrend, &miniMapBoarder);

    SDL_Rect showCurrentView;
    showCurrentView.h = miniMapBoarder.h / 5;
    showCurrentView.w = miniMapBoarder.w / 5;
    showCurrentView.x = game->viewableWorld.centerX;
    showCurrentView.y = game->viewableWorld.centerY;

    SDL_RenderDrawRect(winrend, &showCurrentView);
}

void CreateWorld(SDL_Renderer *winrend, SDL_Texture *background, World *world, int width, int height)
{
    world->height = height;
    world->width = width;
    world->background = background;

    // DO SOMETHING WITH THIS INFO (CREATE BACKGROUND, ect)
}
