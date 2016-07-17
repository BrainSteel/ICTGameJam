

#include "time.h"
#include "stdlib.h"

#include "SDL.h"
#include "GameState.h"
#include "Common.h"
#include "Vector.h"


void Emmit(SDL_Renderer* winrend)
{
    Circle circ;
    Circle circ2;
    Circle circ3;

    srand(time(NULL));
    int r = rand();

    r &= 100;

    int i;
    for(i = 0; i < 10; i++)
    {
        if(r < 10)
        {
            circ.vel.x = 10;
            circ.vel.y = 45;
            circ.acc.x = 1;
            circ.acc.y = 2;

            circ.vel.x = 14;
            circ.vel.y = 23;
            circ.acc.x = 3;
            circ.acc.y = 1;

            circ.vel.x = 20;
            circ.vel.y = 12;
            circ.acc.x = 2;
            circ.acc.y = 5;
        }
        else if(r < 60)
        {
            circ.vel.x = 21;
            circ.vel.y = 23;
            circ.acc.x = 4;
            circ.acc.y = 2;

            circ.vel.x = 12;
            circ.vel.y = 10;
            circ.acc.x = 2;
            circ.acc.y = 2;

            circ.vel.x = 8;
            circ.vel.y = 5;
            circ.acc.x = 1;
            circ.acc.y = 1;
        }
        else
        {
            circ.vel.x = 43;
            circ.vel.y = 12;
            circ.acc.x = 2;
            circ.acc.y = 3;

            circ.vel.x = 14;
            circ.vel.y = 23;
            circ.acc.x = 3;
            circ.acc.y = 1;

            circ.vel.x = 7;
            circ.vel.y = 8;
            circ.acc.x = 1;
            circ.acc.y = 0;
        }


        DrawCircle(winrend, circ, 1);
        DrawCircle(winrend, circ2, 1);
        DrawCircle(winrend, circ3, 1);
    }
}

void BoosterDirection(SDL_Renderer* winrend)
{
    Circle circ;
    circ.pos.x = 100;
    circ.pos.y = 100;
    circ.rad = 20;
    circ.vel.y = 40;
    circ.acc.x = 20;
    circ.acc.y = 20;

    int i;
    for(i = 0; i < 4; i++)
    {
        DrawCircle(winrend, circ, 1);
    }
}

