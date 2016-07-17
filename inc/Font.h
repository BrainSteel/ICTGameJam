//
//  Font.h
//  Just For Nybbles
//
//  Created by Jesse Pritchard on 1/21/15.
//  Copyright (c) 2015 Jesse Pritchard. All rights reserved.
//

#ifndef __Just_For_Nybbles__Font__
#define __Just_For_Nybbles__Font__

#include "SDL.h"

typedef struct _Font{
    SDL_Texture* tex;
    int cw, ch, len;
    int xspace, yspace;
    const char* charlist;
} FNT_Font;

#define FNT_HORIZONTAL 0
#define FNT_VERTICAL 1
#define FNT_ALIGNLEFT (1 << 1)
#define FNT_ALIGNRIGHT (1 << 2)
#define FNT_ALIGNCENTERX (1 << 3)

#define FNT_ALIGNTOP (1 << 4)
#define FNT_ALIGNBOTTOM (1 << 5)
#define FNT_ALIGNCENTERY (1 << 6)

#define FNT_ERROR_NONE 0
#define FNT_ERROR_MEM 1
#define FNT_ERROR_CHAR 2

FNT_Font* FNT_InitFont(SDL_Renderer* rend, const char* file, const char* charlist,
                       int xspace, int yspace, SDL_Color colorkey);
int FNT_DrawText(SDL_Renderer* rend, FNT_Font* font, char* text, int xoff, int yoff, int pxtall, int flags);
void FNT_DestroyFont(FNT_Font* font);

#endif
