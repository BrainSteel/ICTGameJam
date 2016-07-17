#include "Font.h"
#include "SDL.h"

static int FNT_strlen(char* c){
    int i = 0;
    while (*c++ != '\0') i++;
    return i;
}

FNT_Font* FNT_InitFont(SDL_Renderer* rend, const char* file, const char* charlist,
                       int xspace, int yspace, SDL_Color colorkey) {
    FNT_Font* font = NULL;

    SDL_Surface* bmp = SDL_LoadBMP(file);
    if (!bmp) {
        return NULL;
    }
    SDL_SetColorKey(bmp, SDL_TRUE, SDL_MapRGB(bmp->format, colorkey.r, colorkey.g, colorkey.b));
    SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, bmp);
    if (!tex) {
        SDL_FreeSurface(bmp);
        return NULL;
    }
    font = malloc(sizeof(FNT_Font));
    if (!font) {
        SDL_FreeSurface(bmp);
        SDL_DestroyTexture(tex);
        return NULL;
    }
    font->tex = tex;
    font->charlist = charlist;
    font->len = FNT_strlen((char*)charlist);
    font->ch = bmp->h;
    font->cw = bmp->w / font->len;
    font->xspace = xspace;
    font->yspace = yspace;
    SDL_FreeSurface(bmp);

    return font;
}

int FNT_DrawText(SDL_Renderer* rend, FNT_Font* font, char* text, int xoff, int yoff, int pxtall, int flags) {
    if (!rend || !font || !text) {
        return FNT_ERROR_MEM;
    }
    int len = FNT_strlen(text);
    SDL_Rect chr; /* Per-character rectangles on the texture */
    chr.w = font->cw;
    chr.h = font->ch;
    chr.y = 0;

    int w, h; /* Width and height of renderer output */
    if (!((flags & FNT_ALIGNTOP) && (flags & FNT_ALIGNLEFT))) {
        SDL_GetRendererOutputSize(rend, &w, &h);
    }

    SDL_Rect dst; /* Per-character rectangles on the rendering context */
    dst.w = font->cw * (double)(pxtall / font->ch);
    dst.h = pxtall;
    if (flags & FNT_VERTICAL) {
        int totalh = dst.h * len + font->yspace * (len - 1);
        dst.x = xoff;
        if (flags & FNT_ALIGNCENTERX) {
            dst.x = xoff + w / 2 - dst.w / 2;
        }
        else if (flags & FNT_ALIGNRIGHT) {
            dst.x = xoff + w - dst.w;
        }
        else {
            /* Default is left-aligned */
            dst.x = xoff;
        }

        if (flags & FNT_ALIGNCENTERY) {
            dst.y = yoff + h / 2 - totalh / 2;
        }
        else if (flags & FNT_ALIGNBOTTOM) {
            dst.y = yoff + h - totalh;
        }
        else {
            /* Default is top-aligned */
            dst.y = yoff;
        }

        int iii, jjj;
        for (iii = 0; iii < len; iii++) {
            if (text[iii] != ' ') {
                for (jjj = 0; jjj < font->len; jjj++) {
                    if (font->charlist[jjj] == text[iii]) {
                        chr.x = jjj * font->cw;
                        SDL_RenderCopy(rend, font->tex, &chr, &dst);
                        break;
                    }
                }

                if (jjj == font->len) {
                    return FNT_ERROR_CHAR;
                }
            }
            dst.y += dst.h + font->yspace;
        }
    }
    else {
        int totalw = dst.w * len + font->xspace * (len - 1);
        if (flags & FNT_ALIGNCENTERY) {
            dst.y = yoff + h / 2 - dst.h / 2;
        }
        else if (flags & FNT_ALIGNBOTTOM) {
            dst.y = yoff + h - dst.h;
        }
        else {
            /* Default is top-aligned */
            dst.y = yoff;
        }


        if (flags & FNT_ALIGNCENTERX) {
            dst.x = xoff + w / 2 - totalw / 2;
        }
        else if (flags & FNT_ALIGNRIGHT) {
            dst.x = xoff + w - totalw;
        }
        else {
            /* Default is left-aligned */
            dst.x = xoff;
        }

        int iii, jjj;
        for (iii = 0; iii < len; iii++) {
            if (text[iii] != ' ') {
                for (jjj = 0; jjj < font->len; jjj++) {
                    if (font->charlist[jjj] == text[iii]) {
                        chr.x = jjj * font->cw;
                        SDL_RenderCopy(rend, font->tex, &chr, &dst);
                        break;
                    }
                }

                if (jjj == font->len) {
                    return FNT_ERROR_CHAR;
                }
            }
            dst.x += dst.w + font->xspace;
        }
    }

    return FNT_ERROR_NONE;
}

void FNT_DestroyFont(FNT_Font* font) {
    if (font) {
        if (font->tex) {
            SDL_DestroyTexture(font->tex);
        }
        free(font);
    }
}
