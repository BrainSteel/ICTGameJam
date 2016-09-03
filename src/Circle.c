//
// Circle.c
// Small library for circle logic and drawing.
//

#include "SDL.h"

#include "Common.h"
#include "GameState.h"
#include "Vector.h"

void DrawCircle( SDL_Renderer* winrend, Circle circ, int fill ) {
    int drawwidth, drawheight;
    SDL_GetRendererOutputSize( winrend, &drawwidth, &drawheight );

    if (circ.rad == 0) {
        SDL_RenderDrawPoint( winrend, circ.pos.x, circ.pos.y );
        return;
    }

    if (circ.pos.x + circ.rad < 0 || circ.pos.x - circ.rad > drawwidth ||
        circ.pos.y + circ.rad < 0 || circ.pos.y - circ.rad > drawheight )
        return;

#define SHIFT 7
#define SHIFTED_1 ((1 << SHIFT))
    unsigned int circx = circ.pos.x;
    unsigned int circy = circ.pos.y;
    unsigned int x, y;
    x = (int)circ.rad;
    y = 0;

    int e = (x * x + y * y - circ.rad * circ.rad) * SHIFTED_1;
    while ( x >= y ) {
        int xplusx, xminusx, yplusx, yminusx, xplusy, xminusy, yplusy, yminusy;
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

static inline Vector2 CirclePosition( Circle circ, float elapsedtime ) {
    Vector2 result;
    result.x = circ.pos.x + elapsedtime * circ.vel.x + 0.5 * elapsedtime * elapsedtime * circ.acc.x;
    result.y = circ.pos.y + elapsedtime * circ.vel.y + 0.5 * elapsedtime * elapsedtime * circ.acc.y;
    return result;
}

static inline Vector2 CircleVelocity( Circle circ, float elapsedtime ) {
    Vector2 result;
    result.x = circ.vel.x + circ.acc.x * elapsedtime;
    result.y = circ.vel.y + circ.acc.y * elapsedtime;
    return result;
}

static inline void UpdateCirclePosition( Circle* circ, float elapsedtime ) {
    circ->pos = CirclePosition( *circ, elapsedtime );
}

static inline void UpdateCircleVelocity( Circle* circ, float elapsedtime ) {
    circ->vel = CircleVelocity( *circ, elapsedtime );
}

void UpdateCircle( Circle* circ, float elapsedtime ) {
    UpdateCirclePosition( circ, elapsedtime );
    UpdateCircleVelocity( circ, elapsedtime );
}

// This function represents an approximation of collision time by
// evaluating the accurate start and end positions of the circles
// after one unit of time and then evaluating their exact collision
// time had their acceleration in transit between the two locations
// been exactly zero.
// This function is suitable as a first guess for a Newton's method
// approximation, and is most effective for small accelerations.
static float GetCollisionTime( Circle one, Circle two ) {
    // Adjust velocity based on end location
    Vector2 op1 = one.pos;
    UpdateCirclePosition( &one, 1.0 );
    Vector2 ov = VectorSubtract( one.pos, op1 );

    Vector2 tp1 = two.pos;
    UpdateCirclePosition( &two, 1.0 );
    Vector2 tv = VectorSubtract( two.pos, tp1 );

    // Perform the approximation
    float ax = ov.x * ov.x - 2 * ov.x * tv.x + tv.x * tv.x;
    float ay = ov.y * ov.y - 2 * ov.y * tv.y + tv.y * tv.y;
    float bx = 2 * (op1.x * (ov.x - tv.x) + tp1.x * (tv.x - ov.x));
    float by = 2 * (op1.y * (ov.y - tv.y) + tp1.y * (tv.y - ov.y));

    float a = ax + ay;
    float b = bx + by;
    float c = VectorLength2( VectorSubtract( op1, tp1 ));
    float desired = one.rad + two.rad;
    float desired2 = desired * desired;
    c -= desired2;

    float radicand = b * b - 4 * a * c;
    if ( radicand < 0 ) {
        return 0.0 / 0.0;
    }
    else {
        float s = sqrt( radicand );
        float p = s - b;
        float m = -(s + b);
        float top = m < p ? m : p;
        return top / ( 2 * a );
    }
}

static CollisionData GetCollisionDataFromTime( Circle one, Circle two, float coltime, int didoccur ) {
    CollisionData result;
    result.didoccur = didoccur;
    result.elapsedtime = coltime;

    float coltime2 = coltime * coltime;
    result.oneloc.x = one.pos.x + one.vel.x * coltime + 0.5 * one.acc.x * coltime2;
    result.oneloc.y = one.pos.y + one.vel.y * coltime + 0.5 * one.acc.y * coltime2;

    result.twoloc.x = two.pos.x + two.vel.x * coltime + 0.5 * two.acc.x * coltime2;
    result.twoloc.y = two.pos.y + two.vel.y * coltime + 0.5 * two.acc.y * coltime2;

    Vector2 newdist = VectorSubtract( result.twoloc, result.oneloc );
    float ratio = one.rad / (one.rad + two.rad);
    result.loc.x = result.oneloc.x + newdist.x * ratio;
    result.loc.y = result.oneloc.y + newdist.y * ratio;
    return result;
}

static float IterateCollisionTime( Circle* one, Circle* two, float guess, int iterations ) {
    // Use Newton's method to approximate time
    // Using f = (x1 - x2)^2 + (y1 - y2)^2 - (r1 + r2)^2
    // And fprime = 2 * (x1 - x2) * (vx1 - vx2) + 2 * (y1 - y2) * (vy1 - vy2)
    // xnew = xold - f(xold) / fprime(xold)

    float r2 = (one->rad + two->rad) * (one->rad + two->rad);

    float result = guess;
    int i;
    for ( i = 0; i < iterations; i++ ) {
        Vector2 p1 = CirclePosition( *one, result );
        Vector2 v1 = CircleVelocity( *one, result );
        Vector2 p2 = CirclePosition( *two, result );
        Vector2 v2 = CircleVelocity( *two, result );

        float dpx = p1.x - p2.x;
        float dpy = p1.y - p2.y;
        float dvx = v1.x - v2.x;
        float dvy = v1.y - v2.y;

        float f = dpx * dpx + dpy * dpy - r2;
        float fprime = 2 * dpx * dvx + 2 * dpy * dvy;

        result = guess - f / fprime;
    }

    return result;
}


CollisionData GetCollision( Circle one, Circle two, float elapsedtime ) {
    float coltime = GetCollisionTime( one, two );

    if ( one.acc.x != 0 || one.acc.y != 0 ||
         two.acc.x != 0 || two.acc.y != 0 ) {
        coltime = IterateCollisionTime( &one, &two, coltime, 2 );
    }

    if ( coltime == coltime && coltime > 0 && coltime < elapsedtime ) {
        return GetCollisionDataFromTime( one, two, coltime, 1 );
    }
    else {
        CollisionData res;
        res.didoccur = 0;
        return res;
    }
}

typedef struct EntityComponentDataStruct {
    Entity* entity;
    int index;
} EntityComponentData;

static inline float ComponentPosition( float val, void* data ) {
    EntityComponentData* ecdata = data;
    Entity* entity = ecdata->entity;
    Component* comp = &ecdata->entity->components.items[ecdata->index];

    Vector2 pos = comp->relativepos;
    Vector2 vel = entity->body.shape.vel;
    Vector2 acc = entity->body.shape.acc;
    float angacc = entity->angacc;
    float angvel = entity->angvel;

    Vector2 linearpart;
    linearpart.x = pos.x + vel.x * val + 0.5 * acc.x * acc.x * val;
    linearpart.y = pos.y + vel.y * val + 0.5 * acc.y * acc.y * val;

    //Vector2 angularpart;

    return angacc + angvel;
}

static inline float ComponentVelocity( float val, void* data ) {
    return 1;
}

// Represents a more accurate collision estimate between an entity's component and a Circle
// whereby the angular velocity and acceleration of the component around the
CollisionData GetComponentCollision( Entity* entity, int compindex, Circle two, float elapsedtime ) {

}
