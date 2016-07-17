#include "stdlib.h"

#include "SDL.h"

#include "Common.h"
#include "GameState.h"

//#define SCREEN_WIDTH (1920)
#define SCREEN_WIDTH (1280)
//#define SCREEN_HEIGHT (1080)
#define SCREEN_HEIGHT (720)
#define FRAMERATE 40

#define SCALE_FACTOR_X 4
#define SCALE_FACTOR_Y 4

#define MAP_WIDTH (SCREEN_WIDTH * SCALE_FACTOR_X)
#define MAP_HEIGHT (SCREEN_HEIGHT * SCALE_FACTOR_Y)

#define SCALE_FACTOR_X_SQUARED (SCALE_FACTOR_X * SCALE_FACTOR_X)
#define SCALE_FACTOR_Y_SQUARED (SCALE_FACTOR_Y * SCALE_FACTOR_Y)

#define PLAYER_START_X (MAP_WIDTH / 2)
#define PLAYER_START_Y (MAP_HEIGHT / 2)

#define MINIMAP_UPPER_LEFT_X (3 * SCREEN_WIDTH) / 4 //1145
#define MINIMAP_UPPER_LEFT_Y (3 * SCREEN_HEIGHT) / 4 //5

#define HULL_STRENGTH_BAR_W SCREEN_WIDTH / 2

void CreateWorld( SDL_Renderer* winrend, SDL_Texture* background, World* world, int width, int height);

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

    SDL_SetRenderDrawBlendMode(winrend, SDL_BLENDMODE_BLEND);

    SDL_Surface* Background = SDL_LoadBMP("rsc/Background.bmp");
    if(!Background)
    {
        printf("%s", SDL_GetError());
        printf("ERROR-> Background NOT LOADED");
        return 1;
    }

    SDL_Surface* HelpScreen = SDL_LoadBMP(("rsc/HelpScreen.bmp"));
    if(!HelpScreen)
    {
        printf("%s", SDL_GetError());
        printf("ERROR-> HelpScreen NOT LOADED");
        return 1;
    }
    SDL_Texture* HelpScreenTex = SDL_CreateTextureFromSurface(winrend, HelpScreen);

    SDL_PixelFormat* fmt = Background->format;
    Uint8 depth = fmt->BitsPerPixel;
    Uint32 rmask, gmask, bmask, amask;
    rmask = fmt->Rmask;
    gmask = fmt->Gmask;
    bmask = fmt->Bmask;
    amask = fmt->Amask;
    SDL_Surface* BackgroundScaled = SDL_CreateRGBSurface( 0, MAP_WIDTH, MAP_HEIGHT, depth, rmask, gmask, bmask, amask );
    SDL_BlitScaled( Background, NULL, BackgroundScaled, NULL );

    SDL_Surface* Player = SDL_LoadBMP("rsc/Player.bmp");
    if(!Player)
    {
        printf("ERROR-> Player NOT LOADED");
        return 1;
    }
    SDL_SetColorKey(Player, SDL_TRUE, SDL_MapRGB(Player->format, 0, 0, 0));

    SDL_Texture* BackgroundTex = SDL_CreateTextureFromSurface(winrend, BackgroundScaled);
    SDL_Texture* PlayerTex = SDL_CreateTextureFromSurface(winrend, Player);

    SDL_FreeSurface(Background);
    SDL_FreeSurface(BackgroundScaled);
    SDL_FreeSurface(Player);
    SDL_FreeSurface(HelpScreen);

    // Initialize the game state
    GameState* game = GME_InitializeDefault( );
    game->player.Player_TEX = PlayerTex;
    game->player.entity.body.shape.rad = 20;
    game->player.entity.body.shape.pos.x = PLAYER_START_X;
    game->player.entity.body.shape.pos.y = PLAYER_START_Y;
    game->player.entity.body.health = 100;


    game->world.height = MAP_HEIGHT;
    game->world.width = MAP_WIDTH;
    game->world.centerY = MAP_HEIGHT / 2;
    game->world.centerX = MAP_WIDTH / 2;

    game->world.viewableWorld.h = SCREEN_HEIGHT;
    game->world.viewableWorld.w = SCREEN_WIDTH;
    game->world.viewableWorld.x = (MAP_WIDTH / 2) - (SCREEN_WIDTH / 2);
    game->world.viewableWorld.y = (MAP_HEIGHT / 2) - (SCREEN_HEIGHT / 2);

    gamelog( "Creating world ...");
    CreateWorld(winrend, BackgroundTex, &game->world, MAP_WIDTH, MAP_HEIGHT);

    gamelog( "Displaying help screen ..." );
    DisplayHelpScreen( winrend, window, HelpScreenTex, game );


    gamelog( "Running game ..." );
    Run( window, winrend, game );

    gamelog( "Destroying window and renderer ...");
    SDL_DestroyWindow( window );
    SDL_DestroyRenderer( winrend );

    gamelog( "Destroying textures ...");
    SDL_DestroyTexture(BackgroundTex);
    SDL_DestroyTexture(HelpScreenTex);
    FreePlayer( &game->player );

    gamelog( "Quitting SDL ..." );
    SDL_Quit( );
    return 0;
}
// set camera to center of player
int Run( SDL_Window* window, SDL_Renderer* winrend, GameState* game ) {

    double stepw = SCREEN_WIDTH / 10;
    double steph = SCREEN_HEIGHT / 10;

    SDL_Surface* HullStrength_TXT = SDL_LoadBMP("rsc/HullStrength_txt.bmp");
    if(!HullStrength_TXT)
    {
        printf("ERROR-> HullStrength_TXT NOT LOADED");
        return 1;
    }
    SDL_SetColorKey(HullStrength_TXT, SDL_TRUE, SDL_MapRGB(HullStrength_TXT->format, 0, 0, 0));

    SDL_Texture* HullStrength_TXTTex = SDL_CreateTextureFromSurface(winrend, HullStrength_TXT);
    SDL_FreeSurface(HullStrength_TXT);

    SDL_Rect Hull_Strength;
    Hull_Strength.w = stepw * 5;
    Hull_Strength.h = steph / 2;
    Hull_Strength.x = SCREEN_WIDTH / 6;
    Hull_Strength.y = SCREEN_HEIGHT - 40;

    SDL_Rect HullStrengthtxt;
    HullStrengthtxt.w = HULL_STRENGTH_BAR_W;
    HullStrengthtxt.h = steph / 2;
    HullStrengthtxt.x = SCREEN_WIDTH / 6;
    HullStrengthtxt.y = SCREEN_HEIGHT - 40;

    SDL_Rect helpScreenRect;
    helpScreenRect.h = SCREEN_HEIGHT;
    helpScreenRect.w = SCREEN_WIDTH;
    helpScreenRect.x = helpScreenRect.y = 0;



    int HullStrengthDecr = (Hull_Strength.w / 100);

    Hull_Strength.w -= HullStrengthDecr;



    Hull_Strength.w = game->player.entity.body.health;








    SDL_Event event;
    do
    {
        SDL_WaitEvent(&event);
        if ( event.key.keysym.sym == SDLK_ESCAPE) {
                break;
        }

        SDL_SetRenderDrawColor(winrend, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(winrend);

        SDL_RenderCopy(winrend, game->world.helpScreen, NULL, &helpScreenRect);

        SDL_RenderPresent( winrend );

    }while( event.key.keysym.sym != SDLK_SPACE );



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

    Component rocket;
    rocket.strength = 1.0;
    rocket.mass = 2.0;
    rocket.ability = Rocket;
    rocket.health = 10;
    rocket.shape.rad = 10;
    rocket.shape.pos.x = game->player.entity.body.shape.pos.x - 15;
    rocket.shape.pos.y = game->player.entity.body.shape.pos.y + 15;
    Attach( &game->player, rocket );
    rocket.shape.pos.x = game->player.entity.body.shape.pos.x + 15;
    Attach( &game->player, rocket );

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

    gamelog( "Width: %d, Height: %d", game->world.width, game->world.height );

    gamelog( "Waiting for quit event ..." );
    game->frames = 0;
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

        PerformAction( game, Booster );
        PerformAction( game, Rocket );

        UpdatePlayer( game, 1.0 );

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
        else if( (game->world.viewableWorld.x + game->world.viewableWorld.w) > game->world.width )
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

        ////////////////////////////////////////////////


        SDL_SetRenderDrawColor(winrend, 255, 0, 0, SDL_ALPHA_OPAQUE);

        // RENDERING OF THE VISIBLE WORLD PUT ADDTIONAL RENDERS AFTER
        SDL_RenderCopy(winrend, game->world.globalBackground, &game->world.viewableWorld, NULL);

        SDL_SetRenderDrawColor(winrend, 255, 0, 0, 75);
        SDL_RenderDrawRect(winrend, &Hull_Strength);
        SDL_RenderFillRect(winrend, &Hull_Strength);
        SDL_RenderCopy(winrend, HullStrength_TXTTex, NULL, &HullStrengthtxt);

        SDL_SetRenderDrawColor(winrend, 0, 0, 0, SDL_ALPHA_OPAQUE);
        Vector2 offset;
        offset.x = -game->world.viewableWorld.x;
        offset.y = -game->world.viewableWorld.y;
        DrawPlayer( winrend, &game->player, offset );

        SDL_SetRenderDrawColor( winrend, 255, 0, 255, SDL_ALPHA_OPAQUE );
        int i;
        for ( i = 0; i < game->player.numbullet; i++ ) {
            if ( game->player.playerbullets[i].active ) {
                Circle withcontext = game->player.playerbullets[i].shape;
                withcontext.pos.x += offset.x;
                withcontext.pos.y += offset.y;
                DrawCircle( winrend, withcontext, 1 );
            }
        }

        SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );
        SDL_RenderDrawRect(winrend, &miniMap);
        SDL_RenderCopy(winrend, game->world.globalBackground, NULL, &miniMap);
        SDL_RenderDrawRect(winrend, &showCurrentView);

        SDL_RenderPresent( winrend );
        game->frames++;
        uint64_t endtime = SDL_GetTicks( );
        if ( endtime - starttime < 1000 / FRAMERATE) {
            SDL_Delay(( 1000 / FRAMERATE ) - ( SDL_GetTicks() - starttime ));
        }
        else {
            gamelog( "Frame took longer than expected: %llu ms", endtime - starttime );
        }
    }


    SDL_DestroyTexture(HullStrength_TXTTex);

    return 0;
}

void DisplayHelpScreen(SDL_Renderer* winrend, SDL_Window* window, SDL_Texture *helpScreen, GameState* game)
{
    game->world.helpScreen = helpScreen;
}

void CreateWorld(SDL_Renderer *winrend, SDL_Texture *globalBackground, World *world, int width, int height)
{
    world->height = height;
    world->width = width;
    world->globalBackground = globalBackground;

    // DO SOMETHING WITH THIS INFO (CREATE BACKGROUND, ect)
}
