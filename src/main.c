#include "stdlib.h"

#include "SDL.h"

#include "Common.h"
#include "GameState.h"

//#define SCREEN_WIDTH (1920)
#define SCREEN_WIDTH (1920/2)
//#define SCREEN_HEIGHT (1080)
#define SCREEN_HEIGHT (1080/2)
#define FRAMERATE 500

#define SCALE_FACTOR_X 4
#define SCALE_FACTOR_Y 4

#define MAP_WIDTH (SCREEN_WIDTH * SCALE_FACTOR_X)
#define MAP_HEIGHT (SCREEN_HEIGHT * SCALE_FACTOR_Y)

#define SCALE_FACTOR_X_SQUARED (SCALE_FACTOR_X * SCALE_FACTOR_X)
#define SCALE_FACTOR_Y_SQUARED (SCALE_FACTOR_Y * SCALE_FACTOR_Y)

#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 50

#define PLAYER_START_X (MAP_WIDTH / 2)
#define PLAYER_START_Y (MAP_HEIGHT / 2)

#define MINIMAP_UPPER_LEFT_X (3 * SCREEN_WIDTH) / 4 //1145
#define MINIMAP_UPPER_LEFT_Y (3 * SCREEN_HEIGHT) / 4 //5

void CreateWorld( SDL_Renderer* winrend, SDL_Texture* background, SDL_Texture* hiddenBackground, World* world, int width, int height);

int main (int argc, char** argv ) {

    // NOTE: HiddenBackground is totally useless :p

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

    SDL_Surface* HiddenBackground = SDL_LoadBMP("rsc/HiddenBackground.bmp");
    if(!HiddenBackground)
    {
        printf("%s", SDL_GetError());
        printf("ERROR-> HiddenBackground NOT LOADED");
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
    SDL_Texture* HiddenBackgroundTex = SDL_CreateTextureFromSurface(winrend, HiddenBackground);
    SDL_Texture* PlayerTex = SDL_CreateTextureFromSurface(winrend, Player);

    SDL_FreeSurface(Background);
    SDL_FreeSurface(HiddenBackground);
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
    game->world.viewableWorld.y = (MAP_HEIGHT / 2) - (SCREEN_HEIGHT / 2);



    gamelog( "Creating world ...");
    SDL_RenderCopy(winrend, game->world.hiddenBackground, NULL, NULL);
    CreateWorld(winrend, BackgroundTex, HiddenBackgroundTex, &game->world , MAP_WIDTH, MAP_HEIGHT);

    gamelog( "Running game ..." );
    Run( window, winrend, game );

    gamelog( "Destroying window and renderer ...");
    SDL_DestroyWindow( window );
    SDL_DestroyRenderer( winrend );

    gamelog( "Destroying textures ...");
    SDL_DestroyTexture(BackgroundTex);
    SDL_DestroyTexture(HiddenBackgroundTex);
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

    SDL_Rect miniMap;
    miniMap.h = (game->world.viewableWorld.h / SCALE_FACTOR_Y);
    miniMap.w = (game->world.viewableWorld.w / SCALE_FACTOR_X);
    miniMap.x = MINIMAP_UPPER_LEFT_X;
    miniMap.y = MINIMAP_UPPER_LEFT_Y;


    SDL_Rect showCurrentView;
    showCurrentView.h = miniMap.h / SCALE_FACTOR_Y;
    showCurrentView.w = miniMap.w / SCALE_FACTOR_X;
    showCurrentView.x = MINIMAP_UPPER_LEFT_X + ( (PLAYER_START_X / SCALE_FACTOR_X) / SCALE_FACTOR_X );
    showCurrentView.y = MINIMAP_UPPER_LEFT_Y + ( (PLAYER_START_Y / SCALE_FACTOR_Y) / SCALE_FACTOR_Y );

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

        if(game->player.input.keyboard[SDL_SCANCODE_UP] == 1)
        {
            if( game->player.entity.body.shape.pos.y  > game->player.entity.body.shape.rad ){game->player.entity.body.shape.pos.y -= 1;}
        }
        else if(game->player.input.keyboard[SDL_SCANCODE_DOWN] == 1)
        {
            if( game->player.entity.body.shape.pos.y  < (game->world.height - game->player.entity.body.shape.rad) ){game->player.entity.body.shape.pos.y += 1;}
        }
        else if(game->player.input.keyboard[SDL_SCANCODE_RIGHT] == 1)
        {
            if( game->player.entity.body.shape.pos.x < (game->world.width - game->player.entity.body.shape.rad) ) { game->player.entity.body.shape.pos.x += 1; }
        }
        else if(game->player.input.keyboard[SDL_SCANCODE_LEFT] == 1)
        {
            if( game->player.entity.body.shape.pos.x > (game->player.entity.body.shape.rad) ) { game->player.entity.body.shape.pos.x -= 1; }
        }

        game->world.centerX = game->player.entity.body.shape.pos.x;
        game->world.centerX -= game->player.entity.body.shape.vel.x;
        game->world.centerY = game->player.entity.body.shape.pos.y;
        game->world.centerY -= game->player.entity.body.shape.vel.y;

        game->world.viewableWorld.x = game->world.centerX - (SCREEN_WIDTH / 2);
        game->world.viewableWorld.y = game->world.centerY - (SCREEN_HEIGHT / 2);

        if( game->world.viewableWorld.x < 0 )
        {
            game->world.viewableWorld.x = 0;
            game->world.centerX = game->world.viewableWorld.w / 2;
        }
        else if( (game->world.viewableWorld.x + game->world.viewableWorld.w) >  game->world.width )
        {
            game->world.viewableWorld.x = game->world.width - game->world.viewableWorld.w;
            game->world.centerX = game->world.viewableWorld.x + (game->world.viewableWorld.w / 2);
        }
        if( game->world.viewableWorld.y < 0 )
        {
            game->world.viewableWorld.y = 0;
            game->world.centerY = game->world.viewableWorld.h / 2;
        }
        else if( (game->world.viewableWorld.y + game->world.viewableWorld.h > game->world.height) )
        {
            game->world.viewableWorld.y = game->world.height - game->world.viewableWorld.h;
            game->world.centerY = game->world.viewableWorld.y + (game->world.viewableWorld.h / 2);
        }

        showCurrentView.x = miniMap.x + (game->world.viewableWorld.x / SCALE_FACTOR_X_SQUARED);
        showCurrentView.y = miniMap.y + (game->world.viewableWorld.y / SCALE_FACTOR_Y_SQUARED);

        // TO BE USED
        /*
        SDL_Rect HUD_Title;
        HUD_Title.x = 799;
        HUD_Title.y = 50;
        HUD_Title.w = stepw * 20;
        HUD_Title.h = steph * 2;
        */

        SDL_RenderCopy(winrend, game->world.globalBackground, &game->world.viewableWorld, NULL);

        Circle circleOne;

        circleOne.pos.x = game->player.entity.body.shape.pos.x - game->world.viewableWorld.x;
        circleOne.pos.y = game->player.entity.body.shape.pos.y - game->world.viewableWorld.y;
        circleOne.rad = 20;

        SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );

        SDL_RenderDrawRect(winrend, &miniMap);

        SDL_SetRenderDrawColor( winrend, 127, 127, 127, SDL_ALPHA_OPAQUE );
        DrawCircle(winrend, circleOne, 1);
        SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );

        SDL_RenderCopy(winrend, game->world.globalBackground, NULL, &miniMap);


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

void CreateWorld(SDL_Renderer *winrend, SDL_Texture *globalBackground, SDL_Texture *hiddenBackground, World *world, int width, int height)
{
    world->height = height;
    world->width = width;
    world->globalBackground = globalBackground;
    world->hiddenBackground = hiddenBackground;

    // DO SOMETHING WITH THIS INFO (CREATE BACKGROUND, ect)
}
