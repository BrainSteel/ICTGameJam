
#include "xorshiftstar.h"
#include "Common.h"
#include "Vector.h"
#include "GameState.h"

#define MAX_STRENGTH 3

void AddEnemy( GameState* game, int totalstrength ) {
    Enemy* result = NULL;
    int i;
    for ( i = 0; i < game->numenemy; i++ ) {
        Enemy* test = &game->enemies[i];
        if ( !test->alive ) {
            result = test;
            break;
        }
    }

    // Didn't find a dead enemy to replace.
    if ( result == NULL ) {
        Enemy* newenemies = realloc( game->enemies, sizeof(*newenemies) * ( game->numenemy + 1 ));
        if (!newenemies) {
            gamelog( "Ran out of memory allocating enemy ..." );
            return;
        }
        game->enemies = newenemies;
        game->numenemy++;
        result = &game->enemies[game->numenemy - 1];
    }

    // We now have a valid enemy to manipulate.
    int currentstrength = 0;
    int count = 0;

    // TODO > ...

}
