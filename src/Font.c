
#include "Font.h"
#include "SDL.h"

// Find the length of the string
static int FNT_strlen(char* c)
{
    int i = 0;
    while(*c++ != '\0') {i++;}
    return i;
}

FNT_Font* FNT_InitFont(SDL_Renderer* rend, const char* file, const char* charlist,
                       int xSpace, int ySpace, SDL_Color colorkey)
{
    FNT_Font* font = NULL;

    // Create surface, test load, set color key, set tex, free surface
    SDL_Surface* bmp = SDL_LoadBMP(file);
    if(!bmp){return NULL;}
    SDL_SetColorKey(bmp, SDL_TRUE, SDL_MapRGB(bmp->format, colorkey.r, colorkey.g, colorkey.b));
    SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, bmp);
    if(!tex){SDL_FreeSurface(bmp); return NULL;}

    font = malloc(sizeof(FNT_Font));
    if(!font){SDL_FreeSurface(bmp); SDL_DestroyTexture(tex); return NULL;}

    font->tex = tex;
    font->charlist = charlist;
    font->charlen = FNT_strlen((char*)charlist);
    font->charh = bmp->h;
    font->charw = bmp->w / font->charlen;
    font->xSpace = xSpace;
    font->ySpace = ySpace;
    SDL_FreeSurface(bmp);

    return font;
}

int FNT_DrawText(SDL_Renderer* rend, FNT_Font* font, char* text, int xOff, int yOff, int pxTall, int flags)
{
    if(!rend || !font || !text){return FNT_ERROR_MEMORY;}
    int length = FNT_strlen(text);

    SDL_Rect character;
    character.w = font->charw;
    character.h = font->charh;
    character.y = 0;

    int width, height;
    if(!((flags & FNT_ALIGNTOP) && (flags & FNT_ALIGNLEFT))){SDL_GetRendererOutputSize(rend, &width, &height);}

    SDL_Rect destination;
    destination.w = font->charw * (double)(pxTall / font->charh);
    destination.h = pxTall;
    if(flags & FNT_VERTICAL)
    {
        int totalHeight = destination.h * length + font->ySpace * (length - 1);
        destination.x = xOff;
        if(flags & FNT_ALIGNCENTERX){destination.x = xOff + width/2 - destination.w/2;}
        else if(flags & FNT_ALIGNRIGHT){destination.x = xOff + width - destination.w;}
        else {destination.x = xOff;}

        if(flags & FNT_ALIGNCENTERY){destination.y = yOff + height/2 - totalHeight/2;}
        else if(flags & FNT_ALIGNBOTTOM){destination.y = yOff + height - totalHeight;}
        else {destination.y = yOff;}

        int i, j;
        for(i = 0; i < length; i++)
        {
            if(text[i] != ' ')
            {
                for(j = 0; j < font->charlen; j++)
                {
                    if(font->charlist[j] == text[i])
                    {
                        character.x = j * font->charw;
                        SDL_RenderCopy(rend, font->tex, &character, &destination);
                        break;
                    }
                }
                if( j == font->charlen) {return FNT_ERROR_CHAR;}
            }
            destination.y +=destination.h + font->ySpace;
        }
    }
    else
    {
        int totalWidth = destination.w * length + font->xSpace * (length - 1);
        if(flags & FNT_ALIGNCENTERY){destination.y = yOff + height/2 - destination.h/2;}
        else if(flags & FNT_ALIGNBOTTOM){destination.y = yOff + height - destination.h;}
        else{ destination.y = yOff; }

        if(flags & FNT_ALIGNCENTERX){destination.x = xOff + width/2 - totalWidth/2;}
        else if(flags & FNT_ALIGNRIGHT){destination.x = xOff + width - totalWidth;}
        else{destination.x = xOff;}

        int i, j;
        for(i = 0; i < length; i++)
        {
            if(text[i] != ' ')
            {
                for(j = 0; j < font->charlen; j++)
                {
                    if(font->charlist[j] == text[i])
                    {
                        character.x = j * font->charw;
                        SDL_RenderCopy(rend, font->tex, &character, &destination);
                        break;
                    }
                }

                if(j == font->charlen){return FNT_ERROR_CHAR;}
            }
            destination.x =+ destination.w + font->xSpace;
        }
    }

    return FNT_ERROR_NONE;
}

void FNT_DestroyFont(FNT_Font* font)
{
    if(font){if(font->tex){SDL_DestroyTexture(font->tex); free(font);}}
}



