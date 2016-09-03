
#ifndef TEXTURES_H
#define TEXTURES_H

#include "SDL.h"
#include "Common.h"

#define X_TEXTURES                                                      \
X(background, "rsc/Background.bmp", 0, 0, 0, ShapeRect)                 \
X(player, "", 0, 244, 23, ShapeCircle )                                 \
X(enemy, "", 255, 0, 0, ShapeCircle )                                   \
X(comp_armor, "", 255, 255, 255, ShapeCircle )                          \
X(comp_booster, "", 0, 255, 0, ShapeCircle )                            \
X(comp_rocket, "", 0, 0, 255, ShapeCircle )                             \
X(help, "rsc/HelpScreen.bmp", 0, 0, 0, ShapeRect)                       \
X(hullstrengthtext, "rsc/HullStrength_txt.bmp", 0, 0, 0, ShapeRect)     \
X(gameover, "rsc/GameOverScreen.bmp", 0, 0, 0, ShapeRect )

struct DefaultTextureStruct;
struct TextureSetStruct;

typedef enum DefaultShape {
    ShapeCircle,
    ShapeRect
} DefaultShape;

typedef struct sDefaultTexture {
    SDL_Texture* tex;
    SDL_Color c;
    DefaultShape s;
} DefaultTexture;

typedef struct sTextureSet {
#define X( name, str, fr, fg, fb, s ) DefaultTexture name;
    X_TEXTURES
#undef X
} TextureSet;

TextureSet LoadTextures( );
void DrawTexture( SDL_Renderer* rend, DefaultTexture tex, int x, int y );
void DrawTextureEx( SDL_Renderer* rend, DefaultTexture tex, int x, int y, float rot );
void FreeTextures( TextureSet* set );

#endif
