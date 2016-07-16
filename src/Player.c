#include "stdlib.h"
#include "Common.h"
#include "GameState.h"

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
        int last = ref->entity.numcomponent - 1;
        ref->entity.components[last] = pickup;
        ref->entity.components[last].relativepos = VectorSubtract( pickup.shape.pos, ref->entity.body.shape.pos ); 
    }
    else {
        gamelog( "Failed to allocate memory for component." );
    }
}
