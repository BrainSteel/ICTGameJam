
#ifndef FONT_H
#define FONT_H

#include "SDL.h"


typedef struct _Font
{
    SDL_Texture* tex;
    int charw, charh, charlen;
    int xSpace, ySpace;
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
#define FNT_ERROR_MEMORY 1
#define FNT_ERROR_CHAR 2

FNT_Font* FNT_InitFont(SDL_Renderer* rend, const char* file, const char* charlist,
                       int xSpace, int ySpace, SDL_Color colorkey);
int FNT_DrawText(SDL_Renderer* rend, FNT_Font* font, char* text, int xOff, int yOff, int pxTall, int flags);
void FNT_DestroyFont(FNT_Font* font);




#endif // FONT_H
