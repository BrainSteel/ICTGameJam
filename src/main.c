#include "stdlib.h"

#include "SDL.h"

#include "Common.h"
#include "GameState.h"

//#define SCREEN_WIDTH (1920)
#define SCREEN_WIDTH (1920/2)
//#define SCREEN_HEIGHT (1080)
#define SCREEN_HEIGHT (1080/2)
//#define FRAMERATE 40
#define FRAMERATE 2000

#define MAP_WIDTH SCREEN_WIDTH * 5
#define MAP_HEIGHT SCREEN_HEIGHT * 5

#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 50

#define PLAYER_START_X (MAP_WIDTH / 2)
#define PLAYER_START_Y (MAP_HEIGHT / 2)

#define MINIMAP_UPPER_LEFT_X (4 * SCREEN_WIDTH) / 5 //1145
#define MINIMAP_UPPER_LEFT_Y (4 * SCREEN_HEIGHT) / 5 //5

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
    if ( SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN, &window, &winrend) < 0 ) {
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
    game->player.entity.body.shape.rad = 20;
    game->player.entity.body.shape.pos.x = PLAYER_START_X;
    game->player.entity.body.shape.pos.y = PLAYER_START_Y;

    game->world.height = MAP_HEIGHT;
    game->world.width = MAP_WIDTH;
    game->world.centerY = MAP_HEIGHT / 2;
    game->world.centerX = MAP_WIDTH / 2;

    game->world.viewableWorld.h = SCREEN_HEIGHT;
    game->world.viewableWorld.w = SCREEN_WIDTH;
    game->world.viewableWorld.x = (MAP_WIDTH / 2) - (SCREEN_WIDTH / 2);
    game->world.viewableWorld.y = (MAP_HEIGHT / 2) - (SCREEN_HEIGHT / 2)

    gamelog( "Creating world ...");
    CreateWorld(winrend, BackgroundTex, &game->world , MAP_WIDTH, MAP_HEIGHT);

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
// set camera to center of player
int Run( SDL_Window* window, SDL_Renderer* winrend, GameState* game ) {

    // Get window width and height
    int screenwidth, screenheight;
    SDL_GetWindowSize( window, &screenwidth, &screenheight );
    int move_direction = 0;
    SDL_SetRenderDrawColor( winrend, 0, 0, 0, SDL_ALPHA_OPAQUE );

    int playerlocX = PLAYER_START_X;
    int playerlocY = PLAYER_START_Y;

    gamelog( "Clearing white to the screen ...");
    SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );
    SDL_RenderClear( winrend );
    SDL_RenderPresent( winrend );

/*
    SDL_Rect worldSourceSnip;
    worldSourceSnip.h = SCREEN_HEIGHT;
    worldSourceSnip.w = SCREEN_WIDTH;
    worldSourceSnip.x = game->player.entity.body.shape.pos.x - (SCREEN_WIDTH / 2);
    worldSourceSnip.y = game->player.entity.body.shape.pos.y - (SCREEN_HEIGHT / 2);
/*

/*
    SDL_Rect worldDestSnip;
    worldDestSnip.h = game->world.height;
    worldDestSnip.w = game->world.width;
    worldDestSnip.x = 0;
    worldDestSnip.y = 0;
*/

    /*
    SDL_Rect miniMapBoarder;
    miniMapBoarder.h = (game->world.height / 5);
    miniMapBoarder.w = (game->world.width / 5);
    miniMapBoarder.x = MINIMAP_UPPER_LEFT_X;
    miniMapBoarder.y = MINIMAP_UPPER_LEFT_Y;
    */

    SDL_Rect miniMap;
    miniMap.h = (game->world.viewableWorld.h / 5);
    miniMap.w = (game->world.viewableWorld.w / 5);
    miniMap.x = MINIMAP_UPPER_LEFT_X;
    miniMap.y = MINIMAP_UPPER_LEFT_Y;


    SDL_Rect showCurrentView;
    showCurrentView.h = miniMap.h / 5;
    showCurrentView.w = miniMap.w / 5;
    showCurrentView.x = MINIMAP_UPPER_LEFT_X + ( (PLAYER_START_X / 5) / 5 );
    showCurrentView.y = MINIMAP_UPPER_LEFT_Y + ( (PLAYER_START_Y / 5) / 5 );

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

        //(SCREEN_Height / 2)
        //game->player.entity.body.shape.pos.y <= game->world.viewableWorld.x

        if(game->player.input.keyboard[SDL_SCANCODE_UP] == 1)
        {
            if( ( (showCurrentView.y) > miniMap.y) && ( (showCurrentView.y) < (miniMap.y + miniMap.h)) ){game->player.entity.body.shape.pos.y -= 1;}
            if( game->world.viewableWorld.y <= 1 )
            {
                printf("HELLOjijioiHELLO");
                game->world.centerY -= 1;
            }
        }
        else if(game->player.input.keyboard[SDL_SCANCODE_DOWN] == 1)
        {
            if( ( (showCurrentView.y + showCurrentView.h) > miniMap.y) && ( (showCurrentView.y + showCurrentView.h) < (miniMap.y + miniMap.h)) ){game->player.entity.body.shape.pos.y += 1;}
        }
        else if(game->player.input.keyboard[SDL_SCANCODE_RIGHT] == 1)
        {
            if( ( (showCurrentView.x + showCurrentView.w) > miniMap.x) && ( (showCurrentView.x + showCurrentView.w) < (miniMap.x + miniMap.w)) ){game->player.entity.body.shape.pos.x += 1;}
        }
        else if(game->player.input.keyboard[SDL_SCANCODE_LEFT] == 1)
        {
            if( ( (showCurrentView.x) > miniMap.x) && ( (showCurrentView.x) < (miniMap.x + miniMap.w)) ){game->player.entity.body.shape.pos.x -= 1;}
        }


        game->world.centerX = game->player.entity.body.shape.pos.x;
        game->world.centerX -= game->player.entity.body.shape.vel.x;
        game->world.centerY = game->player.entity.body.shape.pos.y;
        game->world.centerY -= game->player.entity.body.shape.vel.y;

        game->world.viewableWorld.x = game->world.centerX - (SCREEN_WIDTH / 2);
        game->world.viewableWorld.y = game->world.centerY - (SCREEN_HEIGHT / 2);

        showCurrentView.x = miniMap.x + (game->world.viewableWorld.x / 25);
        showCurrentView.y = miniMap.y + (game->world.viewableWorld.y / 25);


        //SDL_RenderCopy(winrend, game->world.background, &worldSourceSnip, &worldDestSnip);



        /*
        SDL_Rect playerloc;
        playerloc.h = PLAYER_HEIGHT;
        playerloc.w = PLAYER_WIDTH;
        playerloc.x = playerlocX;
        playerloc.y = playerlocY;
        */

        // TO BE USED
        /*
        SDL_Rect HUD_Title;
        HUD_Title.x = 799;
        HUD_Title.y = 50;
        HUD_Title.w = stepw * 20;
        HUD_Title.h = steph * 2;
        */

/*
        worldSourceSnip.h = SCREEN_HEIGHT;
        worldSourceSnip.w = SCREEN_WIDTH;
        worldSourceSnip.x = game->player.entity.body.shape.pos.x - (SCREEN_WIDTH / 2);
        worldSourceSnip.y = game->player.entity.body.shape.pos.y - (SCREEN_HEIGHT / 2);
*/

        //SDL_RenderCopy(winrend, game->world.globalBackground, &worldSourceSnip, NULL);

        SDL_RenderCopy(winrend, game->world.globalBackground, &game->world.viewableWorld, NULL);

        SDL_RenderCopy(winrend, game->world.globalBackground, NULL, &miniMap);

        Circle circleOne;

        circleOne.pos.x = SCREEN_WIDTH / 2;
        circleOne.pos.y = SCREEN_HEIGHT / 2;
        circleOne.rad = 20;

        SDL_SetRenderDrawColor( winrend, 127, 127, 127, SDL_ALPHA_OPAQUE );
        DrawCircle(winrend, circleOne, 1);
        SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );

        //SDL_RenderCopy(winrend, game->player.Player_TEX, NULL, &playerloc);

        //SDL_RenderDrawRect(winrend, &miniMapBoarder);

        SDL_RenderDrawRect(winrend, &showCurrentView);


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

void CreateWorld(SDL_Renderer *winrend, SDL_Texture *globalBackground, World *world, int width, int height)
{
    world->height = height;
    world->width = width;
    world->globalBackground = globalBackground;

    // DO SOMETHING WITH THIS INFO (CREATE BACKGROUND, ect)
}
