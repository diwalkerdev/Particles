#pragma once
#include "Base/containers/backfill_vector.hpp"
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
    float size;
};

struct Emitter
{
    Vec                           pos;
    Vec                           rot;
    backfill_vector<Particle, 10> particles;
};


void
Emitter_Init(Emitter& emitter)
{
    emitter.pos = Vec { 50.f, 50.f, 0.f };
    emitter.rot = Vec { 0.f, 0.f, 0.f };
}


void
Particle_Init(Particle& particle)
{
    particle.acc          = Vec { 0.f, 0.f, 0.f };
    particle.pos          = Vec { 100.f, 100.f, 0.f };
    particle.life_time_ms = 5000.f;
    particle.duration_ms  = 5000.f;
    particle.size         = 20.f;
}


void
Emitter_Integrate(Emitter& emitter, float time_ms)
{
    // Should Integrate on particles.
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
Emitter_Render(Emitter& emitter, SDL_Renderer* renderer)
{
    SDL_FRect rect { emitter.pos.x,
                     emitter.pos.y,
                     10,
                     10 };

    auto alpha = 255;

    SDL_SetRenderDrawColor(renderer,
                           0,
                           0,
                           255,
                           alpha);

    SDL_RenderDrawRectF(renderer, &rect);
}


void
Particle_Render(Particle& particle, SDL_Renderer* renderer)
{

    SDL_FRect rect { particle.pos.x,
                     particle.pos.y,
                     particle.size,
                     particle.size };

    uint32 alpha = (particle.life_time_ms / particle.duration_ms) * 255;

    SDL_SetRenderDrawColor(renderer,
                           255,
                           0,
                           0,
                           alpha);

    SDL_RenderFillRectF(renderer, &rect);

    // SDL_RenderDrawRectF(renderer, &rect);
}

std::array<Vec, 4> vector_head { { { +0.0, +0.0, +0.0 },
                                   { -0.5, -1.0, +0.0 },
                                   { +0.5, -1.0, +0.0 },
                                   { +0.0, +0.0, +0.0 } } };


template <size_t Size>
void
SRT(std::array<Vec, Size>& U, float scale, float theta, Vec pos)
{
    for (auto& u : U)
    {
        // Scale, Rotate, Translate
        u *= scale;
        u = Vec_Rotate(u, theta);
        u += pos;
    }
}

template <size_t Size>
void
SRT(std::array<Vec, Size>& U, float scale, Rotor2D M, Vec pos)
{
    for (auto& u : U)
    {
        // Scale, Rotate, Translate
        u *= scale;
        u = Vec_Rotate(u, M);
        u += pos;
    }
}


template <size_t Size>
void
Renderer_DrawLines(std::array<Vec, Size> const& lines, SDL_Renderer* renderer)
{
    for (int i = 0; i < lines.size() - 1; ++i)
    {
        auto& from = lines[i];
        auto& to   = lines[(i + 1)];
        SDL_RenderDrawLineF(renderer, from[0], from[1], to[0], to[1]);
    }
}


void
Vector_Render(SDL_Renderer* renderer, Vec const& pos, Vec const& vel)
{
    auto  s = Vec_Magnitude(vel);
    float a = 2;

    std::array<Vec, 2> tail;
    tail[0][0] = 0.0f;
    tail[0][1] = 0.0f;
    tail[0][2] = 0.0f;

    tail[1][0] = 1.0f;
    tail[1][1] = 0.0f;
    tail[1][2] = 0.0f;

    std::array<Vec, 2> cross;
    cross[0][0] = 1.f;
    cross[0][1] = +2.f;
    cross[0][2] = 0.f;

    cross[1][0] = 1.f;
    cross[1][1] = -2.f;
    cross[1][2] = 0.f;

    // Scale the lengths.
    tail[0][0] *= s / a;
    tail[1][0] *= s / a;
    cross[0][0] *= s / a - 2.f; // draw the cross 2 pixels down from the tip.
    cross[1][0] *= s / a - 2.f;

    auto e1 = Vec { 1.0f, 0.0f, 0.0 };
    auto M  = Vec_Mul2D(vel, e1);

    SRT(tail, 1, M, pos);
    SRT(cross, 1, M, pos);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    SDL_FRect base { pos.x - 1.f,
                     pos.y - 1.f,
                     3.f,
                     3.f };
    SDL_RenderFillRectF(renderer, &base);
    Renderer_DrawLines(tail, renderer);
    Renderer_DrawLines(cross, renderer);
}