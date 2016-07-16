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
    CreateWorld(winrend, BackgroundTex, &game->world , 5000, 5000);

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

    int playerlocX = 500;
    int playerlocY = 500;

    gamelog( "Clearing white to the screen ...");
    SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );
    SDL_RenderClear( winrend );
    SDL_RenderPresent( winrend );

    double stepw = SCREEN_WIDTH / 10;
    double steph = SCREEN_HEIGHT / 10;

    gamelog( "Waiting for quit event ..." );
    uint64_t starttime;
    while ( 1 ) {
        starttime = SDL_GetTicks( );

        CaptureInput( game );
        if ( game->quit ) {
            break;
        }

        // THIS IS TERRIBLE
        SDL_Rect worldSourceSnip;
        worldSourceSnip.h = SCREEN_HEIGHT;
        worldSourceSnip.w = SCREEN_WIDTH;
        worldSourceSnip.x = 0;
        worldSourceSnip.h = 0;

        SDL_Rect worldDestSnip;
        worldDestSnip.x = 0;
        worldDestSnip.y = 0;
        worldDestSnip.h = game->viewableWorld.height;
        worldDestSnip.w = game->viewableWorld.width;

        SDL_RenderCopy(winrend, game->world.background, NULL, &worldDestSnip);

        DrawMiniMap(winrend, game);

        SDL_Rect playerloc;
        playerloc.h = PLAYER_HEIGHT;
        playerloc.w = PLAYER_WIDTH;
        playerloc.x = playerlocX;
        playerloc.y = playerlocY;

        SDL_RenderCopy(winrend, game->player.Player_TEX, NULL, &playerloc);

        SDL_Rect HUD_Title;
            HUD_Title.x = 799;
            HUD_Title.y = 50;
            HUD_Title.w = stepw * 20;
            HUD_Title.h = steph * 2;

        if(game->player.input.keyboard[SDL_SCANCODE_UP] == 1)
        {
            game->viewableWorld.centerY -=1;
            playerloc.y = playerlocY -= 1;
            SDL_RenderCopy(winrend, game->player.Player_TEX, NULL, &playerloc);
        }
        else if(game->player.input.keyboard[SDL_SCANCODE_DOWN] == 1)
        {
            game->viewableWorld.centerY +=1;
            playerloc.y = playerlocY += 1;
            SDL_RenderCopy(winrend, game->player.Player_TEX, NULL, &playerloc);
        }
        else if(game->player.input.keyboard[SDL_SCANCODE_RIGHT] == 1)
        {
            game->viewableWorld.centerX +=1;
            playerloc.x = playerlocX += 1;
            SDL_RenderCopy(winrend, game->player.Player_TEX, NULL, &playerloc);
        }
        else if(game->player.input.keyboard[SDL_SCANCODE_LEFT] == 1)
        {
            game->viewableWorld.centerX -=1;
            playerloc.x = playerlocX -= 1;
            SDL_RenderCopy(winrend, game->player.Player_TEX, NULL, &playerloc);
        }

        SDL_RenderPresent(winrend);

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
