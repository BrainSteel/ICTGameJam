//
// main.c
// Entry point and main game loop.
//

#include "stdlib.h"
#include "time.h"
#include "stdio.h"

#include "SDL.h"

#include "GameState.h"
#include "xorshiftstar.h"
#include "Font.h"
#include "Sound.h"

#define SCREEN_WIDTH (1280)
#define SCREEN_HEIGHT (720)
#define FRAMERATE 40
#define SCREEN_LAG_BEHIND 4

#define SCALE_FACTOR_X 4
#define SCALE_FACTOR_Y 4

#define MAP_WIDTH (SCREEN_WIDTH * SCALE_FACTOR_X)
#define MAP_HEIGHT (SCREEN_HEIGHT * SCALE_FACTOR_Y)

#define SCALE_FACTOR_X_SQUARED (SCALE_FACTOR_X * SCALE_FACTOR_X)
#define SCALE_FACTOR_Y_SQUARED (SCALE_FACTOR_Y * SCALE_FACTOR_Y)

#define PLAYER_START_X (MAP_WIDTH / 2)
#define PLAYER_START_Y (MAP_HEIGHT / 2)

#define MINIMAP_UPPER_LEFT_X ((3 * SCREEN_WIDTH) / 4)
#define MINIMAP_UPPER_LEFT_Y ((3 * SCREEN_HEIGHT) / 4)

#define HEALTH_INCREASE_INTERVAL 60

#define COMPONENT_START 20
#define COMPONENT_START_MAX_STR 2

#define ENEMY_START 3
#define ENEMY_MED_STRENGTH 3
#define ENEMY_DIFF_STRENGTH 1
#define SPAWN_RATE 150
#define DIFF_INCREASE 1000

#define HULL_STRENGTH_BAR_W ((SCREEN_WIDTH / 2))

#define PLAYER_MAX_HEALTH 100

#define ElementOffset( type, element ) ((uint64_t)&((type*)0)->element)
#define ValueAtOffset( type, ptr, offset ) (*(type*)(((void*)(ptr))+(offset)))

void InitializeTextures( GameState* game );
int DisplayGameOverScreen(SDL_Renderer* winrend, SDL_Texture *gameoverScreen, GameState* game);
void DisplayHelpScreen(SDL_Renderer* winrend, SDL_Texture *helpScreen, GameState* game);
void CreateWorld( SDL_Renderer* winrend, SDL_Texture* background, World* world, int width, int height);
void DisplayComponentHelpScreen(SDL_Renderer* winrend, SDL_Texture *componentHelpScreen, GameState* game);

int HIGH_SCORE = 0;

int main (int argc, char** argv ) {
    xorshiftseed( time( 0 ));

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

    SDL_Surface* ComponentHelpScreen = SDL_LoadBMP(("rsc/ComponentHelpScreen.bmp"));
    if(!ComponentHelpScreen)
    {
        printf("%s", SDL_GetError());
        printf("ERROR-> ComponentHelpScreen NOT LOADED");
        return 1;
    }
    SDL_Texture* ComponentHelpScreenTex = SDL_CreateTextureFromSurface(winrend, ComponentHelpScreen);


    SDL_Surface* GameOverScreen = SDL_LoadBMP(("rsc/GameOverScreen.bmp"));
    if(!GameOverScreen)
    {
        printf("%s", SDL_GetError());
        printf("ERROR-> GameOverScreen NOT LOADED");
        return 1;
    }
    SDL_Texture* GameOverScreenTex = SDL_CreateTextureFromSurface(winrend, GameOverScreen);

    SDL_PixelFormat* fmt = Background->format;
    Uint8 depth = fmt->BitsPerPixel;
    Uint32 rmask, gmask, bmask, amask;
    rmask = fmt->Rmask;
    gmask = fmt->Gmask;
    bmask = fmt->Bmask;
    amask = fmt->Amask;
    SDL_Surface* BackgroundScaled = SDL_CreateRGBSurface( 0, MAP_WIDTH, MAP_HEIGHT, depth, rmask, gmask, bmask, amask );
    SDL_BlitScaled( Background, NULL, BackgroundScaled, NULL );

    SDL_Texture* BackgroundTex = SDL_CreateTextureFromSurface(winrend, BackgroundScaled);

    SDL_FreeSurface(Background);
    SDL_FreeSurface(BackgroundScaled);
    SDL_FreeSurface(HelpScreen);
    SDL_FreeSurface(GameOverScreen);
    SDL_FreeSurface(ComponentHelpScreen);

    GameState* game;

    // Initialize the game state
    restartLoc:

    game = GME_InitializeDefault( );

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


    //ATM this simply initializes the textures
    gamelog( "Displaying help screen ..." );
    // TODO > Help screen should really be part of Run( )
    DisplayHelpScreen( winrend, HelpScreenTex, game );
    DisplayComponentHelpScreen( winrend, ComponentHelpScreenTex, game );


    gamelog( "Running game ..." );
    Run( window, winrend, game );

    gamelog( "Displaying GameOver Screen ...");
    // TODO > Game over screen should really be part of Run( )
    int result = DisplayGameOverScreen( winrend, GameOverScreenTex, game );

    // TODO > We're not releasing resources here.
    //      > MEMORY LEAK!
    if(result == 2){goto restartLoc;}

    gamelog( "Destroying window and renderer ...");
    SDL_DestroyWindow( window );
    SDL_DestroyRenderer( winrend );

    gamelog( "Destroying textures ...");
    SDL_DestroyTexture(BackgroundTex);
    SDL_DestroyTexture(HelpScreenTex);
    SDL_DestroyTexture(GameOverScreenTex);
    SDL_DestroyTexture(ComponentHelpScreenTex);
    FreePlayer( &game->player );

    gamelog( "Quitting SDL ..." );
    SDL_Quit( );
    return 0;
}

// TODO > This function is way too long and hard to maintain.
int Run( SDL_Window* window, SDL_Renderer* winrend, GameState* game ) {
    int enemymed = ENEMY_MED_STRENGTH;
    int enemydiff = ENEMY_MED_STRENGTH;

    SDL_Color color;
        color.r = 255;
        color.g = 0;
        color.b = 0;

    double stepw = SCREEN_WIDTH / 10;
    double steph = SCREEN_HEIGHT / 10;

    gamelog( "Loading Hull Strength bitmap ..." );
    SDL_Surface* HullStrength_TXT = SDL_LoadBMP("rsc/HullStrength.bmp");
    if(!HullStrength_TXT)
    {
        gamelog( "Failed to load hull strength bitmap: %s ...", SDL_GetError( ) );
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

    // TODO > Configurable/Modifiable/Player-defined starting configuration?
    gamelog( "Initializing player configuration ..." );
    game->player.entity.body.shape.rad = 20;
    game->player.entity.body.shape.pos.x = PLAYER_START_X;
    game->player.entity.body.shape.pos.y = PLAYER_START_Y;
    game->player.entity.body.health = PLAYER_MAX_HEALTH;
    game->player.entity.body.strength = 1.2;
    game->player.entity.body.mass = 1.0;
    game->player.entity.totalmass += game->player.entity.body.mass;
    game->player.entity.body.ability = Booster;
    game->player.entity.body.shape.pos.y = SCREEN_HEIGHT / 2;
    game->player.entity.body.shape.rad = 20;
    game->player.entity.body.health = PLAYER_MAX_HEALTH;
    game->player.entity.body.active = 1;
    Component booster;
    booster.strength = 1.0;
    booster.mass = 1.7;
    booster.ability = Booster;
    booster.health = 10;
    booster.shape.rad = 10;
    booster.shape.pos.x = game->player.entity.body.shape.pos.x + 30;
    booster.shape.pos.y = game->player.entity.body.shape.pos.y;
    Attach( &game->player.entity, booster );
    booster.shape.pos.x = game->player.entity.body.shape.pos.x - 30;
    Attach( &game->player.entity, booster );
    booster.shape.pos.x = game->player.entity.body.shape.pos.x;
    booster.shape.pos.y = game->player.entity.body.shape.pos.y - 30;
    Attach( &game->player.entity, booster );

    Component rocket;
    rocket.strength = 1.0;
    rocket.mass = 2.0;
    rocket.ability = Rocket;
    rocket.health = 10;
    rocket.shape.rad = 10;
    rocket.shape.pos.x = game->player.entity.body.shape.pos.x - 15;
    rocket.shape.pos.y = game->player.entity.body.shape.pos.y + 15;
    Attach( &game->player.entity, rocket );
    rocket.shape.pos.x = game->player.entity.body.shape.pos.x + 15;
    Attach( &game->player.entity, rocket );

    gamelog( "Adding initial enemies ..." );
    int i;
    for (i = 0; i < ENEMY_START; i++) {
        AddEnemy( game, ENEMY_MED_STRENGTH + xorshift64star_uniform(ENEMY_DIFF_STRENGTH * 2 + 1) - ENEMY_DIFF_STRENGTH );
    }

    gamelog( "Adding initial components ..." );
    for ( i = 0; i < COMPONENT_START; i++ ) {
        Component newcomp;
        CMP_InitializeDefault( &newcomp );
        newcomp.shape.pos.x = xorshift64star_uniform( MAP_WIDTH );
        newcomp.shape.pos.y = xorshift64star_uniform( MAP_HEIGHT );
        newcomp.ability = xorshift64star_uniform( NumAbilities );
        newcomp.strength = xorshift64star_uniform( COMPONENT_START_MAX_STR ) + 1;
        FillDataForAbility( &newcomp );
        AddComponent( game, newcomp );
    }

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
    game->frames = 1;
    uint64_t starttime;
    while ( 1 ) {
        starttime = SDL_GetTicks( );

        CaptureInput( game );
        if ( game->quit ) {
            break;
        }

        // Reset player accelerations
        // TODO > Resetting player accelerations right after taking in input doesn't really
        //      > make sense, does it?
        game->player.entity.body.shape.acc.x = 0.0f;
        game->player.entity.body.shape.acc.y = 0.0f;
        game->player.entity.angacc = 0.0f;

        ;
        if ( game->frames % SPAWN_RATE == 0 ) {
            AddEnemy( game, enemymed + xorshift64star_range( -enemydiff, enemydiff ) );
        }

        if ( game->frames % DIFF_INCREASE == 0 ) {
            enemymed += 2;
            enemydiff = enemymed / 10 + 1;
        }

        if ( game->frames % HEALTH_INCREASE_INTERVAL == 0 ) {
            game->player.entity.body.health++;
            if (game->player.entity.body.health > PLAYER_MAX_HEALTH) {
                game->player.entity.body.health = PLAYER_MAX_HEALTH;
            }
        }

        game->player.entity.updateleft = 1.0;
        PerformAction( game, Booster );
        PerformAction( game, Rocket );

        int enemycount;
        for (enemycount = 0; enemycount < game->enemies.num; enemycount++ ) {
            Enemy* enemy = &game->enemies.items[enemycount];
            enemy->entity.updateleft = 1.0;
            if ( enemy->active ) {
                PerformEnemyAction( game, enemy);
            }
        }

        PerformEntityBulletCollisions( game, &game->player.entity, &game->bullets, game->player.entity.updateleft );

        if ( game->player.entity.body.health <= 0 ) {
            break;
        }

        PerformEntityComponentCollisions( game, &game->player.entity, &game->components, game->player.entity.updateleft );

        for (enemycount = 0; enemycount < game->enemies.num; enemycount++ ) {
            Enemy* enemy = &game->enemies.items[enemycount];
            if ( enemy->active ) {
                PerformEntityBulletCollisions( game, &enemy->entity, &game->player.bullets, enemy->entity.updateleft );
                if ( enemy->entity.body.health <= 0 ) {
                    ManagedListFree( Component, &enemy->entity.components );
                    ManagedListDeactivate( Enemy, &game->enemies, enemycount );
                }

                PerformEntityComponentCollisions(game, &enemy->entity, &game->components, enemy->entity.updateleft );
            }
        }


        UpdateEntity( game, &game->player.entity, game->player.entity.updateleft );

        for ( enemycount = 0; enemycount < game->enemies.num; enemycount++ ) {
            Enemy* enemy = &game->enemies.items[enemycount];
            if ( enemy->active ) {
                enemy->entity.updateleft = 1.0;
                UpdateEntity( game, &enemy->entity, enemy->entity.updateleft );
            }
        }


        int compcount;
        for ( compcount = 0; compcount < game->components.num; compcount++ ) {
            Component* comp = &game->components.items[compcount];
            if ( comp->active ) {
                if ( comp->invinceframes >= 0 ) {
                    // TODO > Figure out why the hell I put this here
                    comp->invinceframes -= 1 > comp->invinceframes ? comp->invinceframes : 1;
                }
                UpdateCircle( &comp->shape, 1.0 );
                float rad = comp->shape.rad;
                Vector2 newpos = comp->shape.pos;

                // If the circle went out of bounds, we should deactivate this component
                if ( newpos.x < -rad || newpos.y < -rad ) {
                    ManagedListDeactivate( Component, &game->components, compcount );
                }
                else if ( newpos.x > MAP_WIDTH + rad || newpos.y > MAP_HEIGHT + rad ) {
                    ManagedListDeactivate( Component, &game->components, compcount );
                }
            }
        }

        int bulcount;
        for ( bulcount = 0; bulcount < game->player.bullets.num; bulcount++ ) {
            Bullet* bul = &game->player.bullets.items[bulcount];
            if ( bul->active ) {
                // Move all active bullets 1 frame
                UpdateCircle( &bul->shape, 1.0 );
                bul->lifetime -= 1.0;
                Vector2 bulpos = bul->shape.pos;
                float bulrad = bul->shape.rad;
                if (bul->lifetime <= 0) {
                    // If the lifetime has expired, deactivate the bullet
                    ManagedListDeactivate( Bullet, &game->player.bullets, bulcount );
                }
                // if the bullet went out of bounds, deactivate the bullet
                else if ( bulpos.x < -bulrad || bulpos.y < -bulrad ) {
                    ManagedListDeactivate( Bullet, &game->player.bullets, bulcount );
                }
                else if ( bulpos.x > MAP_WIDTH + bulrad || bulpos.y > MAP_HEIGHT + bulrad ) {
                    ManagedListDeactivate( Bullet, &game->player.bullets, bulcount );
                }
            }
        }

        for ( bulcount = 0; bulcount < game->bullets.num; bulcount++ ) {
            Bullet* bul = &game->bullets.items[bulcount];
            if ( bul->active ) {
                // Move all active bullets 1 frame
                UpdateCircle( &bul->shape, 1.0 );
                bul->lifetime -= 1.0;
                Vector2 bulpos = bul->shape.pos;
                float bulrad = bul->shape.rad;
                if (bul->lifetime <= 0) {
                    // If the lifetime has expired, deactivate the bullet
                    ManagedListDeactivate( Bullet, &game->bullets, bulcount );
                }
                // if the bullet went out of bounds, deactivate the bullet
                else if ( bulpos.x < -bulrad || bulpos.y < -bulrad ) {
                    ManagedListDeactivate( Bullet, &game->bullets, bulcount );
                }
                else if ( bulpos.x > MAP_WIDTH + bulrad || bulpos.y > MAP_HEIGHT + bulrad ) {
                    ManagedListDeactivate( Bullet, &game->bullets, bulcount );
                }
            }
        }

        game->world.centerX = game->player.entity.body.shape.pos.x;
        game->world.centerX -= SCREEN_LAG_BEHIND * game->player.entity.body.shape.vel.x;
        game->world.centerY = game->player.entity.body.shape.pos.y;
        game->world.centerY -= SCREEN_LAG_BEHIND * game->player.entity.body.shape.vel.y;

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
        Hull_Strength.w = HULL_STRENGTH_BAR_W;
        SDL_RenderDrawRect(winrend, &Hull_Strength);
        Hull_Strength.w = (game->player.entity.body.health * HULL_STRENGTH_BAR_W) / PLAYER_MAX_HEALTH;
        SDL_RenderFillRect(winrend, &Hull_Strength);
        Hull_Strength.w = HULL_STRENGTH_BAR_W;
        SDL_RenderCopy(winrend, HullStrength_TXTTex, NULL, &HullStrengthtxt);
        SDL_SetRenderDrawColor(winrend, 0, 0, 0, SDL_ALPHA_OPAQUE);
        Vector2 offset;
        offset.x = -game->world.viewableWorld.x;
        offset.y = -game->world.viewableWorld.y;

        DrawEntity( winrend, &game->player.entity, offset );

        SDL_SetRenderDrawColor( winrend, 255, 0, 255, SDL_ALPHA_OPAQUE );
        int i;

        // TODO > Separate drawing functions and modifiable colors for
        //      > bullets and components?
        for ( i = 0; i < game->player.bullets.num; i++ ) {
            if ( game->player.bullets.items[i].active ) {
                Circle withcontext = game->player.bullets.items[i].shape;
                withcontext.pos.x += offset.x;
                withcontext.pos.y += offset.y;
                DrawCircle( winrend, withcontext, 1 );
            }
        }

        SDL_SetRenderDrawColor( winrend, 255, 255, 0, SDL_ALPHA_OPAQUE );
        for ( i = 0; i < game->bullets.num; i++ ) {
            if ( game->bullets.items[i].active ) {
                Circle withcontext = game->bullets.items[i].shape;
                withcontext.pos.x += offset.x;
                withcontext.pos.y += offset.y;
                DrawCircle( winrend, withcontext, 1);
            }
        }

        for ( i = 0; i < game->components.num; i++ ) {
            if ( game->components.items[i].active ) {
                DrawComponent(winrend, &game->components.items[i], offset);
            }
        }

        SDL_SetRenderDrawColor( winrend, 255, 255, 0, SDL_ALPHA_OPAQUE );
        for ( i = 0; i < game->enemies.num; i++ ) {
            if ( game->enemies.items[i].active ) {
                DrawEntity( winrend, &game->enemies.items[i].entity, offset );
            }
        }

        SDL_RenderCopy(winrend, game->world.globalBackground, NULL, &miniMap);
        SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );
        SDL_RenderDrawRect(winrend, &miniMap);
        SDL_RenderDrawRect(winrend, &showCurrentView);

        SDL_SetRenderDrawColor( winrend, 255, 100, 100, SDL_ALPHA_OPAQUE );
        for ( i = 0; i < game->enemies.num; i++ ) {
            if ( game->enemies.items[i].active ) {
                Circle circ;
                circ.rad = 2;
                circ.pos.x = game->enemies.items[i].entity.body.shape.pos.x / SCALE_FACTOR_X_SQUARED + miniMap.x;
                circ.pos.y = game->enemies.items[i].entity.body.shape.pos.y / SCALE_FACTOR_Y_SQUARED + miniMap.y;
                DrawCircle( winrend, circ, 1 );
            }
        }

        SDL_RenderPresent( winrend );
        game->frames++;

        // TODO > Variable frame rate!
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

int DisplayGameOverScreen(SDL_Renderer* winrend, SDL_Texture *gameoverScreen, GameState* game)
{
    SDL_Color color;
    color.r = 255;
    color.g = 0;
    color.b = 0;

    FNT_Font* font = FNT_InitFont(winrend, "rsc/540x20Font.bmp", "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 3, 4, color);


    game->world.gameoverScreen = gameoverScreen;

    SDL_Rect gameoverScreenRect;
    gameoverScreenRect.h = SCREEN_HEIGHT;
    gameoverScreenRect.w = SCREEN_WIDTH;
    gameoverScreenRect.x = gameoverScreenRect.y = 0;

    SDL_Rect restartButtonRect;
    restartButtonRect.h = 90;
    restartButtonRect.w = 365;
    restartButtonRect.x = 195;
    restartButtonRect.y = 360;

    SDL_Rect quitButtonRect;
    quitButtonRect.h = 90;
    quitButtonRect.w = 365;
    quitButtonRect.x = 790;
    quitButtonRect.y = 360;

    SDL_Event event;
    do
    {
        SDL_WaitEvent(&event);
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            if( (event.button.x > restartButtonRect.x && event.button.x < restartButtonRect.x + restartButtonRect.w) && (event.button.y > restartButtonRect.y && event.button.y < restartButtonRect.y + restartButtonRect.h) )
            {
                ManagedListFree( Bullet, &game->bullets );
                ManagedListFree( Component, &game->components );

                int i;
                for(i = 0; i < game->enemies.num; i++)
                {
                    FreeEnemy(( &game->enemies.items[i] ));
                }
                ManagedListFree( Enemy, &game->enemies );
                FreePlayer( &game->player );

                return 2;

            }
            else if( (event.button.x > quitButtonRect.x && event.button.x < quitButtonRect.x + quitButtonRect.w) && (event.button.y > quitButtonRect.y && event.button.y < quitButtonRect.y + quitButtonRect.h) )
            {
                break;
            }
        }


        game->player.SCORE = game->frames / (FRAMERATE / 2);

        if(game->player.SCORE > HIGH_SCORE)
        {
            HIGH_SCORE = game->player.SCORE;
        }

        char buf[100];
        char buf2[100];

        sprintf(buf, "%d", game->player.SCORE);

        sprintf(buf2, "%d", HIGH_SCORE);

        SDL_SetRenderDrawColor(winrend, 255, 255, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(winrend);

        SDL_RenderCopy(winrend, game->world.gameoverScreen, NULL, &gameoverScreenRect);
        SDL_RenderDrawRect(winrend, &restartButtonRect);
        SDL_RenderDrawRect(winrend, &quitButtonRect);

        FNT_DrawText(winrend, font, "SCORE", 50, 250, 30, FNT_ALIGNLEFT | FNT_ALIGNTOP);
        FNT_DrawText(winrend, font, buf, 150, 250, 30, FNT_ALIGNLEFT | FNT_ALIGNTOP);


        FNT_DrawText(winrend, font, "HIGHSCORE", 50, 300, 30, FNT_ALIGNLEFT | FNT_ALIGNTOP);
        FNT_DrawText(winrend, font, buf2, 225, 300, 30, FNT_ALIGNLEFT | FNT_ALIGNTOP);

        SDL_RenderPresent( winrend );

    }while( event.key.keysym.sym != SDLK_SPACE );

    FNT_DestroyFont( font );

    return 1;

}

// TODO > We could use a more useful InitializePlayer routine.

// TODO > This function does not make sense.
void DisplayHelpScreen(SDL_Renderer* winrend, SDL_Texture *helpScreen, GameState* game)
{
    game->world.helpScreen = helpScreen;

    SDL_Rect helpScreenRect;
    helpScreenRect.h = SCREEN_HEIGHT;
    helpScreenRect.w = SCREEN_WIDTH;
    helpScreenRect.x = helpScreenRect.y = 0;


    SDL_Event event;
    do
    {
        SDL_WaitEvent(&event);

        if( event.key.keysym.sym == SDLK_ESCAPE) {break;}

        SDL_SetRenderDrawColor(winrend, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(winrend);

        SDL_RenderCopy(winrend, game->world.helpScreen, NULL, &helpScreenRect);

        SDL_RenderPresent( winrend );

    }while( event.key.keysym.sym != SDLK_SPACE );
}

void DisplayComponentHelpScreen(SDL_Renderer* winrend, SDL_Texture *componentHelpScreen, GameState* game)
{
    game->world.componentHelpScreen = componentHelpScreen;

    SDL_Rect helpScreenRect;
    helpScreenRect.h = SCREEN_HEIGHT;
    helpScreenRect.w = SCREEN_WIDTH;
    helpScreenRect.x = helpScreenRect.y = 0;

    SDL_Event event;

    do
    {
        SDL_WaitEvent(&event);
        if ( event.key.keysym.sym == SDLK_ESCAPE) {break;}

        SDL_SetRenderDrawColor(winrend, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(winrend);

        SDL_RenderCopy(winrend, game->world.componentHelpScreen, NULL, &helpScreenRect);

        SDL_RenderPresent( winrend );

    }while( event.key.keysym.sym != SDLK_ESCAPE );

}

// TODO > This function doesn't really do anything.
void CreateWorld(SDL_Renderer *winrend, SDL_Texture *globalBackground, World *world, int width, int height)
{
    world->height = height;
    world->width = width;
    world->globalBackground = globalBackground;

    // DO SOMETHING WITH THIS INFO (CREATE BACKGROUND, ect)
}
