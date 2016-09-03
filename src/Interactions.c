//
// Interactions.c
// File to handle interactions between the player,
// entities, bullets, and components.
//

#include "stdlib.h"

#include "Common.h"
#include "Vector.h"
#include "GameState.h"
#include "xorshiftstar.h"

typedef enum eCollisionType {
    BulletAndEntity,
    ComponentAndEntity,
    BulletAndComponent,
    ComponentAndComponent
} CollisionType;

typedef struct sEntityCollision{
    CollisionData col;
    void* item1;
    void* item2;
    CollisionType type;
} EntityCollision;

static int CompareCollisions( const void* a, const void* b ) {
    const EntityCollision* ac = a;
    const EntityCollision* bc = b;
    if ( ac == NULL && bc == NULL ) {
        return 0;
    }
    else if ( ac == NULL ) {
        return 1;
    }
    else if ( bc == NULL ) {
        return -1;
    }

    if ( ac->col.didoccur && bc->col.didoccur ) {
        if ( ac->col.elapsedtime < bc->col.elapsedtime ) {
            return -1;
        }
        else if ( ac->col.elapsedtime > bc->col.elapsedtime ) {
            return 1;
        }
        else {
            return 0;
        }
    }
    else if ( ac->col.didoccur ) {
        return -1;
    }
    else if ( bc->col.didoccur ) {
        return 1;
    }
    return 0;
}

static void PerformOneEntityComponentCollision( GameState* game, Entity* entity,
                                        ManagedList( Component )* clist, EntityCollision ecol ) {
    if ( !ecol.col.didoccur ) {
        return;
    }

    Component* ocomp = NULL;
    if ( ecol.type == ComponentAndEntity ) {
        if ( ecol.item1 != entity ) {
            gamelog( "Collision item 1 and related entity are not equal." );
            return;
        }

        ocomp = ecol.item2;
    }
    else if ( ecol.type == ComponentAndComponent ) {
        ocomp = ecol.item2;
    }
    else {
        gamelog( "Unrecognized collision type: %d", ecol.type );
    }

    UpdateEntity( game, entity, ecol.col.elapsedtime );
    if ( ocomp->active ) {
        UpdateCircle( (Circle*)ocomp, ecol.col.elapsedtime );
        if ( ocomp->invinceframes <= 0 ) {
            Attach( entity, *ocomp );
            ManagedListDeactivate( Component, clist, ocomp - clist->items );
        }
    }
}

// TODO > This function is quite inefficient.
//      > We should detect !didoccur's earlier so they
//      > don't take forever to sort/use.
void PerformEntityComponentCollisions( GameState* game, Entity* entity,
                                       ManagedList( Component )* components, float elapsedtime ) {

    // This is potentially expensive
    unsigned totalnum = (entity->components.num + 1) * components->num;
    EntityCollision* all = malloc( sizeof( *all ) * totalnum );

    int colindex = 0;
    int i, j;
    // Get all entity->component + component collisions
    for ( i = 0; i < entity->components.num; i++ ) {
        for ( j = 0; j < components->num; j++, colindex++ ) {
            if ( entity->components.items[i].active && components->items[j].active ) {
                Circle entcomp = entity->components.items[i].shape;
                entcomp.vel.x = entity->body.shape.vel.x;
                entcomp.vel.y = entity->body.shape.vel.y;
                all[colindex].item1 = &entity->components.items[i];
                all[colindex].item2 = &components->items[j];
                all[colindex].type = ComponentAndComponent;
                all[colindex].col = GetCollision( entcomp, components->items[j].shape, elapsedtime );
            }
            else {
                all[colindex].col.didoccur = 0;
            }
        }
    }

    // Get all entity + component collisions
    for ( j = 0; j < components->num; j++, colindex++ ) {
        if ( components->items[j].active ) {
            all[colindex].item1 = entity;
            all[colindex].item2 = &components->items[j];
            all[colindex].type = ComponentAndEntity;
            all[colindex].col = GetCollision( entity->body.shape, components->items[j].shape, elapsedtime );
        }
        else {
            all[colindex].col.didoccur = 0;
        }
    }

    // Sort them in order of time that they happen
    qsort( all, totalnum, sizeof( *all ), CompareCollisions );

    // Update the entity and components through all of the collisions
    float elapsedyet = 0.0;
    for ( colindex = 0; colindex < totalnum; colindex++ ) {
        if ( !all[colindex].col.didoccur ) {
            break;
        }

        all[colindex].col.elapsedtime -= elapsedyet;
        PerformOneEntityComponentCollision( game, entity, components, all[colindex] );
        elapsedyet += all[colindex].col.elapsedtime;
    }

    entity->updateleft -= elapsedyet;

    free( all );
}

#define COMPONENT_LAUNCHV 5.0
#define RETRIEVABLE_UPPER 1
#define RETRIEVABLE_SUCCESS 0

// TODO > This function is quite inefficient.
//      > We should detect !didoccur's earlier so they
//      > don't take forever to sort/use.
void PerformEntityBulletCollisions( GameState* game, Entity* entity, ManagedList( Bullet )* bullets, float elapsedtime ) {
    // This is potentially expensive
    unsigned totalnum = (entity->components.num + 1) * bullets->num;
    EntityCollision* all = malloc( sizeof( *all ) * totalnum );

    int colindex = 0;
    int i, j;
    // Get all entity->component + bullet collisions
    for ( i = 0; i < entity->components.num; i++ ) {
        for ( j = 0; j < bullets->num; j++, colindex++ ) {
            if ( entity->components.items[i].active && bullets->items[j].active ) {
                Circle entcomp = entity->components.items[i].shape;
                entcomp.vel.x = entity->body.shape.vel.x;
                entcomp.vel.y = entity->body.shape.vel.y;
                all[colindex].item1 = &entity->components.items[i];
                all[colindex].item2 = &bullets->items[j];
                all[colindex].type = BulletAndComponent;
                all[colindex].col = GetCollision( entcomp, bullets->items[j].shape, elapsedtime );
            }
            else {
                all[colindex].col.didoccur = 0;
            }
        }
    }

    // Get all entity + bullet collisions
    for ( j = 0; j < bullets->num; j++, colindex++ ) {
        if ( bullets->items[j].active ) {
            all[colindex].item1 = entity;
            all[colindex].item2 = &bullets->items[j];
            all[colindex].type = BulletAndEntity;
            all[colindex].col = GetCollision( entity->body.shape, bullets->items[j].shape, elapsedtime );
        }
        else {
            all[colindex].col.didoccur = 0;
        }
    }

    // Sort them in order of time that they happen
    qsort( all, totalnum, sizeof( *all ), CompareCollisions );

    for ( i = 0; i < totalnum; i++ ) {
        if ( !all[i].col.didoccur ) {
            break;
        }

        Component* comp = all[i].item1;
        Bullet* bul = all[i].item2;
        if ( bul->active ) {
            comp->health -= bul->damage;
            if ( comp->health <= 0 ) {
                if ( all[i].type == BulletAndComponent ) {
                    if (xorshift64star_uniform( RETRIEVABLE_UPPER ) <= RETRIEVABLE_SUCCESS ) {
                        Component copy = *comp;
                        Vector2 dir = VectorNormalize( copy.relativepos );
                        copy.shape.vel = VectorScale( dir, COMPONENT_LAUNCHV );
                        AddComponent( game, copy );
                    }
                    Detach( entity, comp - entity->components.items );
                }
                else if ( all[i].type == BulletAndEntity ) {
                    int compcount;
                    for ( compcount = 0; compcount < entity->components.num; compcount++ ) {
                        if ( entity->components.items[compcount].active ) {
                            if (xorshift64star_uniform( RETRIEVABLE_UPPER ) <= RETRIEVABLE_SUCCESS ) {
                                Component copy = entity->components.items[compcount];
                                Vector2 dir = VectorNormalize( copy.relativepos );
                                copy.shape.vel = VectorScale( dir, COMPONENT_LAUNCHV );
                                AddComponent( game, copy );
                            }
                            Detach( entity, compcount );
                        }
                    }
                }
                else {
                    gamelog( "Unrecognized type for Bullet collision: %d", all[i].type );
                }
            }
            ManagedListDeactivate( Bullet, bullets, bul - bullets->items );
        }
    }

    free( all );
}
