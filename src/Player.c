#include "stdlib.h"
#include "Common.h"
#include "GameState.h"

#define MAX_PLAYER_SPEED 14
#define MAX_ANG_VELOCITY 0.3

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

void Attach( Player* ref, Component pickup ) {
    Component* newlist = realloc( ref->entity.components, sizeof(*newlist) * ( ref->entity.numcomponent + 1 ));
    if ( newlist ) {
        ref->entity.components = newlist;
        ref->entity.numcomponent++;
        int last = ref->entity.numcomponent - 1;
        ref->entity.components[last] = pickup;
        ref->entity.components[last].relativepos = VectorSubtract( pickup.shape.pos, ref->entity.body.shape.pos );
        float r = VectorLength( ref->entity.components[last].relativepos );
        ref->entity.MOI += ref->entity.components[last].mass * r * r;
        ref->entity.totalmass += ref->entity.components[last].mass;
    }
    else {
        gamelog( "Failed to allocate memory for component." );
    }
}

void UpdatePlayer( Player* player, float elapsedtime ) {
    float elapsedtime2 = elapsedtime * elapsedtime;
    Circle* circ = &player->entity.body.shape;

    circ->pos.x += circ->vel.x * elapsedtime + 0.5 * circ->acc.x * elapsedtime2;
    circ->pos.y += circ->vel.y * elapsedtime + 0.5 * circ->acc.y * elapsedtime2;

    float ang = player->entity.angvel * elapsedtime;
    float cosang = cos( ang );
    float sinang = sin( ang );

    int i;
    for ( i = 0; i < player->entity.numcomponent; i++ ) {
        Component* comp = &player->entity.components[i];

        // Rotation matrix
        Vector2 rotated;
        rotated.x = cosang * comp->relativepos.x + sinang * comp->relativepos.y;
        rotated.y = cosang * comp->relativepos.y - sinang * comp->relativepos.x;

        comp->relativepos.x = rotated.x;
        comp->relativepos.y = rotated.y;
        comp->shape.pos.x = circ->pos.x + rotated.x;
        comp->shape.pos.y = circ->pos.y + rotated.y;
    }

    player->entity.angvel += player->entity.angacc * elapsedtime;
    if (player->entity.angvel > MAX_ANG_VELOCITY) {
        player->entity.angvel = MAX_ANG_VELOCITY;
    }
    else if (player->entity.angvel < -MAX_ANG_VELOCITY) {
        player->entity.angvel = -MAX_ANG_VELOCITY;
    }

    circ->vel.x += circ->acc.x * elapsedtime;
    circ->vel.y += circ->acc.y * elapsedtime;

    if ( circ->vel.x * circ->vel.x + circ->vel.y * circ->vel.y > MAX_PLAYER_SPEED * MAX_PLAYER_SPEED ) {
        circ->vel = VectorScale( VectorNormalize( circ->vel ), MAX_PLAYER_SPEED );
    }
}

void DrawPlayer( SDL_Renderer* winrend, Player* player, Vector2 offset ) {
    // Maintain previous draw color
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor( winrend, &r, &g, &b, &a );

    // Draw the body in red
    SDL_SetRenderDrawColor( winrend, 255, 0, 0, SDL_ALPHA_OPAQUE );
    DrawCircle( winrend, player->entity.body.shape, 1 );

    // Draw all components
    int i;
    for ( i = 0; i < player->entity.numcomponent; i++ ) {
        Component* comp = &player->entity.components[i];
        switch ( comp->ability ) {
            case None:
                SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );
                break;
            case Rocket:
                SDL_SetRenderDrawColor( winrend, 0, 0, 255, SDL_ALPHA_OPAQUE );
                break;
            case Booster:
                SDL_SetRenderDrawColor( winrend, 0, 255, 0, SDL_ALPHA_OPAQUE );
                break;
        }

        DrawCircle( winrend, comp->shape, 1 );
    }

    SDL_SetRenderDrawColor( winrend, r, g, b, a );
}

static void useBoosters( Player* player );
static void useRockets( Player* player );

void PerformAction( Player* player, AbilityType action ) {
    switch (action) {
        case Booster:
            useBoosters( player );
            break;
        case Rocket:
            useRockets( player );
            break;
        default:
            break;
    }
}

#define ROOT2_2 0.70710678118f

static void useBoosters( Player* player ) {

    int right = 1;
    int up = 1 << 1;
    int left = 1 << 2;
    int down = 1 << 3;

    int direction = 0;
    int count = 0;
    if (player->input.keyboard[SDL_SCANCODE_UP]) {
        direction |= up;
        count++;
    }
    else if (player->input.keyboard[SDL_SCANCODE_DOWN]) {
        direction |= down;
        count++;
    }

    if (player->input.keyboard[SDL_SCANCODE_RIGHT]) {
        direction |= right;
        count++;
    }
    else if (player->input.keyboard[SDL_SCANCODE_LEFT]) {
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

    int i;
    for ( i = 0; i < player->entity.numcomponent; i++ ) {
        Component* comp = &player->entity.components[i];
        if ( comp->ability == Booster ) {
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

static void useRockets( Player* player ) {

}

void FreePlayer( Player* player ) {
    if ( player->entity.components != NULL ) {
        free( player->entity.components );
    }
    SDL_DestroyTexture( player->Player_TEX );
    free( player );
}
