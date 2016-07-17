#include "stdlib.h"
#include "Common.h"
#include "GameState.h"

#define MAX_PLAYER_SPEED 10
#define MAX_ANG_VELOCITY 0.2

static Bullet* UseFirstInactiveBullet( Player* player );
static void AllocBulletBatch( Player* player );
static void DeactivateBullet( Bullet* bullet );

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

void Attach( Entity* entity, Component pickup ) {
    int index;
    for ( index = 0; index < entity->numcomponent; index++ ) {
        if ( entity->components[index].health <= 0) {
            break;
        }
    }

    if (index == entity->numcomponent) {
        Component* newlist = realloc( entity->components, sizeof(*newlist) * ( entity->numcomponent + 1 ));
        if ( newlist ) {
            entity->components = newlist;
            entity->numcomponent++;
        }
        else {
            gamelog( "Failed to allocate memory for component." );
        }
    }

    entity->components[index] = pickup;
    entity->components[index].relativepos = VectorSubtract( pickup.shape.pos, entity->body.shape.pos );
    float r = VectorLength( entity->components[index].relativepos );
    entity->MOI += entity->components[index].mass * r * r;
    entity->totalmass += entity->components[index].mass;
}

void UpdateEntity( GameState* game, Entity* entity, float elapsedtime ) {
    float elapsedtime2 = elapsedtime * elapsedtime;
    Circle* circ = &entity->body.shape;

    circ->pos.x += circ->vel.x * elapsedtime + 0.5 * circ->acc.x * elapsedtime2;
    circ->pos.y += circ->vel.y * elapsedtime + 0.5 * circ->acc.y * elapsedtime2;

    if ( circ->pos.x < 0 ) {
        circ->pos.x = 0;
        circ->vel.x = 0;
    }
    else if ( circ->pos.x > game->world.width - circ->rad) {
        circ->pos.x = game->world.width - circ->rad;
        circ->vel.x = 0;
    }

    if ( circ->pos.y < 0 ) {
        circ->pos.y = 0;
        circ->vel.y = 0;
    }
    else if ( circ->pos.y > game->world.height - circ->rad ) {
        circ->pos.y = game->world.height - circ->rad;
        circ->vel.y = 0;
    }

    float ang = entity->angvel * elapsedtime;
    float cosang = cos( ang );
    float sinang = sin( ang );

    int i;
    for ( i = 0; i < entity->numcomponent; i++ ) {
        Component* comp = &entity->components[i];

        // Rotation matrix
        Vector2 rotated;
        rotated.x = cosang * comp->relativepos.x + sinang * comp->relativepos.y;
        rotated.y = cosang * comp->relativepos.y - sinang * comp->relativepos.x;

        comp->relativepos.x = rotated.x;
        comp->relativepos.y = rotated.y;
        comp->shape.pos.x = circ->pos.x + rotated.x;
        comp->shape.pos.y = circ->pos.y + rotated.y;
    }

    entity->angvel += entity->angacc * elapsedtime;
    if (entity->angvel > MAX_ANG_VELOCITY) {
        entity->angvel = MAX_ANG_VELOCITY;
    }
    else if (entity->angvel < -MAX_ANG_VELOCITY) {
        entity->angvel = -MAX_ANG_VELOCITY;
    }

    circ->vel.x += circ->acc.x * elapsedtime;
    circ->vel.y += circ->acc.y * elapsedtime;

    if ( circ->vel.x * circ->vel.x + circ->vel.y * circ->vel.y > MAX_PLAYER_SPEED * MAX_PLAYER_SPEED ) {
        circ->vel = VectorScale( VectorNormalize( circ->vel ), MAX_PLAYER_SPEED );
    }
}

void UpdatePlayer( GameState* game, float elapsedtime ) {
    Player* player = &game->player;

    UpdateEntity( game, &player->entity, elapsedtime );

    int i;
    for ( i = 0; i < player->numbullet; i++ ) {
        Bullet* bullet = &player->playerbullets[i];
        if ( bullet->active ) {
            UpdateCircle( &bullet->shape, elapsedtime );
            bullet->lifetime -= elapsedtime;
            if (bullet->lifetime <= 0) {
                DeactivateBullet( bullet );
                if ( i < player->firstinactivebullet ) {
                    player->firstinactivebullet = i;
                }
            }
        }
    }
}

static void DeactivateBullet( Bullet* bullet ) {
    bullet->active = 0;
}

void DrawEntity( SDL_Renderer* winrend, Entity* entity, Vector2 offset ) {
    if ( entity->body.health <= 0 )
        return;

    // Maintain previous draw color
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor( winrend, &r, &g, &b, &a );

    // If friend, draw the body in dark green.
    // Else, red.
    if ( entity->type == Friend ) {
        SDL_SetRenderDrawColor( winrend, 10, 128, 30, SDL_ALPHA_OPAQUE );
    }
    else {
        SDL_SetRenderDrawColor( winrend, 255, 0, 0, SDL_ALPHA_OPAQUE );
    }

    Circle withoffset = entity->body.shape;
    withoffset.pos.x += offset.x;
    withoffset.pos.y += offset.y;
    DrawCircle( winrend, withoffset, 1 );

    // Draw all components
    int i;
    for ( i = 0; i < entity->numcomponent; i++ ) {
        Component* comp = &entity->components[i];
        if (comp->health <= 0)
            continue;
        DrawComponent( winrend, comp, offset );
    }

    SDL_SetRenderDrawColor( winrend, r, g, b, a );
}

void DrawComponent( SDL_Renderer* winrend, Component* comp, Vector2 offset ) {
    if (comp->health <= 0)
        return;

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
        default:
            SDL_SetRenderDrawColor( winrend, 255, 255, 255, SDL_ALPHA_OPAQUE );
    }

    Circle withoffset = comp->shape;
    withoffset.pos.x += offset.x;
    withoffset.pos.y += offset.y;
    DrawCircle( winrend, withoffset, 1 );
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

    int i;
    for ( i = 0; i < player->entity.numcomponent; i++ ) {
        Component* comp = &player->entity.components[i];
        if ( comp->ability == Booster && comp->health > 0 ) {
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
        for ( i = 0; i < game->player.entity.numcomponent; i++ ) {
            Component* comp = &game->player.entity.components[i];
            if ( comp->ability == Rocket &&
                 comp->health > 0 &&
                 (game->frames - comp->frameused) > ROCKET_CONSTANT - ROCKET_CONSTANT_FACTOR * comp->strength ) {
                Bullet* newbullet = UseFirstInactiveBullet( &game->player );
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

static Bullet* UseFirstInactiveBullet( Player* player ) {
    if ( player->numbullet <= player->firstinactivebullet ) {
        // Assume this works because time.
        AllocBulletBatch( player );

        Bullet* result = &player->playerbullets[player->firstinactivebullet];

        // We just allocated new memory, so all of the bullets hereafter are inactive.
        player->firstinactivebullet++;
        return result;
    }
    else {
        Bullet* result = &player->playerbullets[player->firstinactivebullet];
        for ( player->firstinactivebullet++; player->firstinactivebullet < player->numbullet; player->firstinactivebullet++ ) {
            if ( !player->playerbullets[player->firstinactivebullet].active ) {
                break;
            }
        }

        if ( player->firstinactivebullet == player->numbullet ) {
            AllocBulletBatch( player );
        }

        return result;
    }
}

#define BATCH_SIZE 10
static void AllocBulletBatch( Player* player ) {
    Bullet* bullets = realloc( player->playerbullets, (player->numbullet + BATCH_SIZE) * sizeof(*bullets) );
    if ( bullets ) {
        player->playerbullets = bullets;
        int i;
        for ( i = player->numbullet; i < player->numbullet + BATCH_SIZE; i++ ) {
            BLT_InitializeDefault( &player->playerbullets[i] );
        }
        player->numbullet += BATCH_SIZE;
    }
}

void FreePlayer( Player* player ) {
    if ( player->entity.numcomponent > 0 ) {
        free( player->entity.components );
    }
    if ( player->numbullet > 0 ) {
        free( player->playerbullets );
    }
    SDL_DestroyTexture( player->Player_TEX );
}
