//
// Player.c
// File to handle player interactions with the game.
// This file contains functions that will handle input,
// and use that input to update and move the player figure.
//

#include "stdlib.h"
#include "Common.h"
#include "GameState.h"

DefineManagedListFunctions( Bullet );

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
    state->player.input.mousebutton = SDL_GetMouseState( &x, &y );
    state->player.input.mouseloc.x = x;
    state->player.input.mouseloc.y = y;
}

void UpdatePlayer( GameState* game, float elapsedtime ) {
    Player* player = &game->player;

    UpdateEntity( game, &player->entity, elapsedtime );

    int i;
    for ( i = 0; i < player->bullets.num; i++ ) {
        Bullet* bullet = &player->bullets.items[i];
        if ( bullet->active ) {
            UpdateCircle( &bullet->shape, elapsedtime );
            bullet->lifetime -= elapsedtime;
            if (bullet->lifetime <= 0) {
                ManagedListDeactivate( Bullet, &player->bullets, i );
            }
        }
    }
}

static void UseBoosters( Player* player );
static void UseRockets( GameState* player );

void PerformAction( GameState* game, AbilityType action ) {
    switch (action) {
        case Booster:
            UseBoosters( &game->player );
            break;
        case Rocket:
            UseRockets( game );
            break;
        default:
            break;
    }
}

static void UseBoosters( Player* player ) {

    int right = 1;
    int up = 1 << 1;
    int left = 1 << 2;
    int down = 1 << 3;

    int direction = 0;
    int count = 0;
    if (player->input.keyboard[SDL_SCANCODE_W]) {
        direction |= up;
        count++;
    }
    else if (player->input.keyboard[SDL_SCANCODE_S]) {
        direction |= down;
        count++;
    }

    if (player->input.keyboard[SDL_SCANCODE_D]) {
        direction |= right;
        count++;
    }
    else if (player->input.keyboard[SDL_SCANCODE_A]) {
        direction |= left;
        count++;
    }

    if ( count == 0 ) {
        return;
    }

    float mod = 1.0f;
    if ( count == 2 ) {
        mod = ROOT2_2;
    }

    Vector2 linearforce;
    VCT_InitializeDefault( &linearforce );
    float moment = 0.0f;

    Component* body = &player->entity.body;
    if (!body->active) return;

    if (body->ability == Booster) {
        if ( direction & up ) {
            linearforce.y -= mod * body->strength;
        }
        else if ( direction & down ) {
            linearforce.y += mod * body->strength;
        }

        if ( direction & right ) {
            linearforce.x += mod * body->strength;
        }
        else if ( direction & left ) {
            linearforce.x -= mod * body->strength;
        }
    }

    int i;
    for ( i = 0; i < player->entity.components.num; i++ ) {
        Component* comp = &player->entity.components.items[i];
        if ( comp->ability == Booster && comp->active ) {
            if ( direction & up ) {
                linearforce.y -= mod * comp->strength;
                moment += comp->relativepos.x * mod * comp->strength;
            }
            else if ( direction & down ) {
                linearforce.y += mod * comp->strength;
                moment -= comp->relativepos.x * mod * comp->strength;
            }

            if ( direction & right ) {
                linearforce.x += mod * comp->strength;
                moment += comp->relativepos.y * mod * comp->strength;
            }
            else if ( direction & left ) {
                linearforce.x -= mod * comp->strength;
                moment -= comp->relativepos.y * mod * comp->strength;
            }
        }
    }

    player->entity.body.shape.acc.x = linearforce.x / player->entity.totalmass;
    player->entity.body.shape.acc.y = linearforce.y / player->entity.totalmass;
    player->entity.angacc = moment / player->entity.MOI;
}

static void UseRockets( GameState* game ) {
    if ( game->player.input.mousebutton & SDL_BUTTON( SDL_BUTTON_LEFT )) {
        int i;
        for ( i = 0; i < game->player.entity.components.num; i++ ) {
            Component* comp = &game->player.entity.components.items[i];
            if ( comp->ability == Rocket &&
                 comp->active &&
                 (game->frames - comp->frameused) > ROCKET_CONSTANT - ROCKET_CONSTANT_FACTOR * comp->strength ) {
                Bullet* newbullet = ManagedListUseFirstInactive( Bullet, &game->player.bullets );
                newbullet->active = 1;
                newbullet->damage = PLAYER_BULLET_DAMAGE;
                newbullet->lifetime = PLAYER_BULLET_LIFETIME;
                newbullet->shape.pos = comp->shape.pos;
                Vector2 mouseincontext;
                mouseincontext.x = game->player.input.mouseloc.x + game->world.viewableWorld.x;
                mouseincontext.y = game->player.input.mouseloc.y + game->world.viewableWorld.y;
                Vector2 dir = VectorNormalize( VectorSubtract( mouseincontext, comp->shape.pos ));
                newbullet->shape.vel = VectorScale( dir, PLAYER_BULLET_SPEED );
                newbullet->shape.vel.x += game->player.entity.body.shape.vel.x;
                newbullet->shape.vel.y += game->player.entity.body.shape.vel.y;

                comp->frameused = game->frames;

                newbullet->shape.rad = PLAYER_BULLET_RADIUS * sqrt(comp->strength);
                newbullet->shape.acc.x = 0.0f;
                newbullet->shape.acc.y = 0.0f;
            }
        }
    }
}

void FreePlayer( Player* player ) {
    ManagedListFree( Bullet, &player->bullets );
    ManagedListFree( Component, &player->entity.components );
    SDL_DestroyTexture( player->Player_TEX );
}
