#include "SDL.h"

#include "Common.h"
#include "GameState.h"
#include "Vector.h"

void DrawCircle( SDL_Renderer* winrend, Circle circ, int fill ) {
    int drawwidth, drawheight;
    SDL_GetRendererOutputSize( winrend, &drawwidth, &drawheight );
#define SHIFT 7
#define SHIFTED_1 ((1 << 7))
    unsigned int circx = circ.pos.x;
    unsigned int circy = circ.pos.y;
    unsigned int x, y;
    x = (int)circ.rad;
    y = 0;

    int e = (x * x + y * y - circ.rad * circ.rad) * SHIFTED_1;
    while ( x >= y ) {
        unsigned int xplusx, xminusx, yplusx, yminusx, xplusy, xminusy, yplusy, yminusy;
        xplusx = (circx + x);
        xminusx = (circx - x);
        yplusx = (circy + x);
        yminusx = (circy - x);
        xplusy = (circx + y);
        xminusy = (circx - y);
        yplusy = (circy + y);
        yminusy = (circy - y);

        if ( !fill ) {
            SDL_RenderDrawPoint( winrend, xplusx, yplusy );
            SDL_RenderDrawPoint( winrend, xminusx, yplusy );
            SDL_RenderDrawPoint( winrend, xplusx, yminusy );
            SDL_RenderDrawPoint( winrend, xminusx, yminusy );
            SDL_RenderDrawPoint( winrend, xplusy, yplusx );
            SDL_RenderDrawPoint( winrend, xminusy, yplusx );
            SDL_RenderDrawPoint( winrend, xplusy, yminusx );
            SDL_RenderDrawPoint( winrend, xminusy, yminusx );
        }
        else {
            SDL_RenderDrawLine( winrend, xplusx, yplusy, xminusx, yplusy );
            SDL_RenderDrawLine( winrend, xplusx, yminusy, xminusx, yminusy );
            SDL_RenderDrawLine( winrend, xplusy, yplusx, xminusy, yplusx );
            SDL_RenderDrawLine( winrend, xplusy, yminusx, xminusy, yminusx );
        }

        y += 1;
        e += 2 * y * SHIFTED_1 + SHIFTED_1;
        if ( e > SHIFTED_1 ) {
            e += SHIFTED_1 - 2 * x * SHIFTED_1;
            x -= 1;
        }
    }
}

CollisionData GetCollision( Circle one, Circle two, float elapsedtime ) {

    float ax = one.vel.x * one.vel.x - 2 * one.vel.x * two.vel.x + two.vel.x * two.vel.x;
    float ay = one.vel.y * one.vel.y - 2 * one.vel.y * two.vel.y + two.vel.y * two.vel.y;
    float bx = 2 * (one.pos.x * (one.vel.x - two.vel.x) + two.pos.x * (two.vel.x - one.vel.x));
    float by = 2 * (one.pos.y * (one.vel.y - two.vel.y) + two.pos.y * (two.vel.y - one.vel.y));

    float a = ax + ay;
    float b = bx + by;
    float c = VectorLength2( VectorSubtract( one.pos, two.pos ));
    float desired = one.rad + two.rad;
    float desired2 = desired * desired;
    c -= desired2;

    CollisionData result;
    float radicand = b * b - 4 * a * c;
    if ( radicand < 0 ) {
        result.didoccur = 0;
        return result;
    }
    else {
        float s = sqrt( radicand );
        float top;
        float p = s - b;
        float m = -(s + b);
        if (p > 0 && m > 0) {
            top = m < p ? m : p;
        }
        else if (p > 0) {
            top = p;
        }
        else if (m > 0) {
            top = m;
        }
        else {
            result.didoccur = 0;
            return result;
        }

        float ans = top / ( 2 * a );
        if ( ans < elapsedtime ) {
            result.didoccur = 1;
            result.elapsedtime = ans;

            float ans2 = ans * ans;
            result.oneloc.x = one.pos.x + one.vel.x * ans + 0.5 * one.acc.x * ans2;
            result.oneloc.y = one.pos.y + one.vel.y * ans + 0.5 * one.acc.y * ans2;

            result.twoloc.x = two.pos.x + two.vel.x * ans + 0.5 * two.acc.x * ans2;
            result.twoloc.y = two.pos.y + two.vel.y * ans + 0.5 * two.acc.y * ans2;

            Vector2 newdist = VectorSubtract( result.twoloc, result.oneloc );
            float ratio = one.rad / (one.rad + two.rad);
            result.loc.x = result.oneloc.x + newdist.x * ratio;
            result.loc.y = result.oneloc.y + newdist.y * ratio;
            return result;
        }
        else {
            result.didoccur = 0;
            return result;
        }
    }

}

void UpdateCircle( Circle* circ, float elapsedtime ) {
    circ->pos.x += circ->vel.x * elapsedtime + 0.5 * circ->acc.x * elapsedtime * elapsedtime;
    circ->pos.y += circ->vel.y * elapsedtime + 0.5 * circ->acc.y * elapsedtime * elapsedtime;
    circ->vel.x += circ->acc.x * elapsedtime;
    circ->vel.y += circ->acc.y * elapsedtime;
}
