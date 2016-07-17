

#include "time.h"
#include "stdlib.h"

#include "SDL.h"
#include "GameState.h"
#include "Common.h"
#include "Vector.h"

void UpdateParticle( Circle particle, float elapsedtime );

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
    // To simulate flying UP -> Particles will move down
    Circle simMoveUp;
    simMoveUp.pos.x = 100;
    simMoveUp.pos.y = 100;
    simMoveUp.rad = 2;
    simMoveUp.vel.x = 0;
    simMoveUp.vel.y = 40;
    simMoveUp.acc.x = 0;
    simMoveUp.acc.y = 20;

    // To simulate flying DOWN -> Particles will move up
    Circle simMoveDown;
    simMoveDown.pos.x = 100;
    simMoveDown.pos.y = 100;
    simMoveDown.rad = 2;
    simMoveDown.vel.x = 0;
    simMoveDown.vel.y = -40;
    simMoveDown.acc.x = 0;
    simMoveDown.acc.y = -20;

    // To simulate flying RIGHT -> Particles will move left
    Circle simMoveRight;
    simMoveRight.pos.x = 100;
    simMoveRight.pos.y = 150;
    simMoveRight.rad = 2;
    simMoveRight.vel.x = -40;
    simMoveRight.vel.y = 0;
    simMoveRight.acc.x = -20;
    simMoveRight.acc.y = 0;

    // To simulate flying LEFT -> Particles will move right
    Circle simMoveLeft;
    simMoveLeft.pos.x = 100;
    simMoveLeft.pos.y = 180;
    simMoveLeft.rad = 2;
    simMoveLeft.vel.x = 40;
    simMoveLeft.vel.y = 0;
    simMoveLeft.acc.x = 20;
    simMoveLeft.acc.y = 0;

    int i;
    for(i = 0; i < 4; i++)
    {

        /*DrawCircle(winrend, simMoveRight, 1);
        UpdateCircle(&simMoveRight, 1.0);
        DrawCircle(winrend, simMoveLeft, 1);
        UpdateCircle(&simMoveLeft, 1.0);*/
        DrawCircle(winrend, simMoveUp, 1);
        UpdateCircle(&simMoveUp, 1.0);
        //DrawCircle(winrend, simMoveDown, 1);
        //UpdateCircle(&simMoveDown, 1.0);

        UpdateParticle(simMoveUp, 1.0);
    }
}
//GameState* game,
void UpdateParticle( Circle particle, float elapsedtime )
{
    GameState game;

    particle.pos.x = particle.vel.x * elapsedtime + 0.5 * particle.acc.x;
    particle.pos.y = particle.vel.y * elapsedtime + 0.5 * particle.acc.y;
    /*
    if ( particle.pos.x < 0 ) {
        particle.pos.x = 0;
        particle.vel.x = 0;
    }
    else if ( particle.pos.x > game->world.width - particle.rad) {
        particle.pos.x = game->world.width - particle.rad;
        particle.vel.x = 0;
    }

    if ( particle.pos.y < 0 ) {
        particle.pos.y = 0;
        particle.vel.y = 0;
    }
    else if ( particle.pos.y > game->world.height - particle.rad ) {
        particle.pos.y = game->world.height - particle.rad;
        particle.vel.y = 0;
    }*/
}
