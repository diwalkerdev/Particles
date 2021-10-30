#pragma once
#include "Base/typedefs.h"
#include "GeometricAlgebra/geometric_algebra.h"
#include <SDL2/SDL.h>


// Where there is one, there are many.
struct Particle
{
    Vec   acc;
    Vec   pos;
    float life_time_ms;
    float duration_ms;
    float rad;
};


void
Particle_Init(Particle& particle)
{
    particle.acc          = Vec { 0.f, 0.f, 0.f };
    particle.pos          = Vec { 100.f, 100.f, 0.f };
    particle.life_time_ms = 5000.f;
    particle.duration_ms  = 5000.f;
    particle.rad          = 20.f;
}


void
Particle_Integrate(Particle& particle, float time_ms)
{
    particle.life_time_ms -= time_ms;
    if (particle.life_time_ms < 0)
    {
        particle.life_time_ms = 0;
    }
}


void
Particle_Render(Particle& particle, SDL_Renderer* renderer)
{

    SDL_FRect rect { particle.pos.x,
                     particle.pos.y,
                     particle.rad,
                     particle.rad };

    uint32 alpha = (particle.life_time_ms / particle.duration_ms) * 255;

    SDL_SetRenderDrawColor(renderer,
                           255,
                           0,
                           0,
                           alpha);

    SDL_RenderFillRectF(renderer, &rect);

    // SDL_RenderDrawRectF(renderer, &rect);
}
