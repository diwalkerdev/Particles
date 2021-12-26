#pragma once
#include "Base/containers/backfill_vector.hpp"
#include "Base/typedefs.h"
#include "GeometricAlgebra/geometric_algebra.h"
#include <SDL2/SDL.h>
#include <stdlib.h>


// Where there is one, there are many.
struct Particle
{
    Vec     acc;
    Vec     vel;
    Vec     pos;
    Vec     theta;
    Vec     omega;
    Vec     alpha;
    Matrix4 rot_mat;
    float   lifetime_sec;
    float   duration_sec;
    float   size;
};


struct Emitter
{
    backfill_vector<Particle, 40> particles;

    // TODO(DW): A count down timer would be nice.
    float rate { 0.5f };
    float timer { 0.5f };
};

enum class Property_Tag
{
    Float,
    SizeT,
};

struct Property_Float
{
    float* ptr;
    float  value;
    float  min, max;
};

struct Property_SizeT
{
    size_t* ptr;
    size_t  value;
    size_t  min, max;
};

struct Property
{
    Property_Tag tag;
    char const*  name;
    bool         read_only;
    union
    {
        Property_Float prop_float;
        Property_SizeT prop_sizet;
    };
};


std::vector<Property>
Emitter_Properties(Emitter& emitter)
{
    std::vector<Property> properties;

    Property prop1 = {
        .tag        = Property_Tag::SizeT,
        .name       = "Watermark",
        .read_only  = true,
        .prop_sizet = { nullptr,
                        emitter.particles.size(),
                        0,
                        emitter.particles.capacity() }
    };

    Property prop2 = {
        .tag        = Property_Tag::Float,
        .name       = "Rate",
        .read_only  = false,
        .prop_float = { .ptr   = &emitter.rate,
                        .value = 0,
                        .min   = 0.2,
                        .max   = 2 }
    };

    properties.push_back(prop1);
    properties.push_back(prop2);

    return properties;
}


void
Particle_Init(Particle& particle)
{
    particle.pos = Vec { 0.0f, 0.0f, 0.f };
    // particle.vel = Vec { 5.f, 10.f, 0.f };
    particle.vel = Vec { 0.f, 0.f, 0.f };
    particle.acc = Vec { 0.f, 0.f, 0.f };

    // NOTE(DW): Remember there's a double integration. So you need to add
    // some multiple of 60.0f (the FPS), to achieve the desired rad/sec.
    particle.theta = Vec { 0.0f, 0.0f, 0.f };
    particle.omega = Vec { 0.0f, 0.0f, 0.f };
    particle.alpha = Vec { 0.0f, 0.0f, 0.0f };

    particle.lifetime_sec = 5.0f;
    particle.duration_sec = 5.0f;
    particle.size         = 20.0f;
}


void
Particle_Integrate(Particle& particle, float time_sec)
{
    particle.lifetime_sec -= time_sec;
    if (particle.lifetime_sec < 0)
    {
        return;
    }

    auto t  = time_sec;
    auto kg = 1.0f;
    // auto m  = 1.0f / kg;
    auto g = Vec { 0.f, -9.81f, 0.f };

    particle.acc += ((g * kg) * t);
    particle.vel += particle.acc * t;
    particle.pos += particle.vel * t;

    particle.theta.x += particle.omega.x / 60.0f;
    particle.theta.y += particle.omega.y / 60.0f;
    particle.theta.z += particle.omega.z / 60.0f;

    // float theta_e12 = particle.omega.x;
    // float theta_e13 = particle.omega.y;
    // float theta_e23 = particle.omega.z;
    float theta_e12 = particle.theta.x;
    float theta_e13 = particle.theta.y;
    float theta_e23 = particle.theta.z;

    auto R = RotorFromEuler(theta_e13, theta_e23, theta_e12);

    particle.rot_mat = ToMatrix4(R);
}


void
Emitter_Init(Emitter& emitter)
{
}


void
Emitter_Integrate(Emitter& emitter, float time_sec)
{
    emitter.timer -= time_sec;
    if (emitter.timer < 0.0f)
    {
        emitter.timer = emitter.rate;
        emitter.particles.allocate();
        auto& particle = emitter.particles.back();
        Particle_Init(particle);

        auto deg = rand() % 360 - 180;
        auto rad = deg * M_PI / 180.0f;
        auto R   = RotorFromEuler(rad, 0, 0);

        auto u       = Vec { 5.f, 10.f, 0.f };
        u            = Vec_Rotate(R, u);
        particle.vel = u;

        auto  d1       = rand() % 360 - 180;
        auto  d2       = rand() % 360 - 180;
        auto  d3       = rand() % 360 - 180;
        float r1       = d1 * M_PI / 180.0f;
        float r2       = d2 * M_PI / 180.0f;
        float r3       = d3 * M_PI / 180.0f;
        particle.omega = Vec { r1, r2, r3 };
    }

    for (auto& particle : emitter.particles)
    {
        Particle_Integrate(particle, time_sec);
    }

    std::vector<size_t> to_remove;
    for (size_t i = 0; i < emitter.particles.size(); ++i)
    {
        auto& particle = emitter.particles[i];
        if (particle.lifetime_sec < 0)
        {
            to_remove.push_back(i);
        }
    }
    emitter.particles.remove(to_remove);
}


// void
// Emitter_Render(Emitter& emitter, SDL_Renderer* renderer)
// {
//     SDL_FRect rect { emitter.pos.x,
//                      emitter.pos.y,
//                      10,
//                      10 };

//     auto alpha = 255;

//     SDL_SetRenderDrawColor(renderer,
//                            0,
//                            0,
//                            255,
//                            alpha);

//     SDL_RenderDrawRectF(renderer, &rect);
// }


// void
// Particle_Render(Particle& particle, Renderer& renderer)
// {
//     SDL_FRect rect { particle.pos.x,
//                      TO_EUCLID(renderer, particle.pos.y),
//                      particle.size,
//                      particle.size };

//     uint32 alpha = (particle.life_time_ms / particle.duration_ms) * 255;

//     SDL_SetRenderDrawColor(renderer.renderer,
//                            255,
//                            0,
//                            0,
//                            alpha);

//     SDL_RenderFillRectF(renderer.renderer, &rect);

//     // SDL_RenderDrawRectF(renderer, &rect);
// }


// template <size_t Size>
// void
// SRT(std::array<Vec, Size>& U, float scale, float theta, Vec pos)
// {
//     for (auto& u : U)
//     {
//         // Scale, Rotate, Translate
//         u *= scale;
//         u = Vec_Rotate(u, theta);
//         u += pos;
//     }
// }


// template <size_t Size>
// void
// SRT(std::array<Vec, Size>& U, float scale, Rotor2D M, Vec pos)
// {
//     for (auto& u : U)
//     {
//         // Scale, Rotate, Translate
//         u *= scale;
//         u = Vec_Rotate(u, M);
//         u += pos;
//     }
// }


// template <size_t Size>
// void
// Renderer_DrawLines(std::array<Vec, Size> const& lines, Renderer& renderer)
// {
//     for (int i = 0; i < lines.size() - 1; ++i)
//     {
//         auto& from = lines[i];
//         auto& to   = lines[(i + 1)];
//         SDL_RenderDrawLineF(renderer.renderer,
//                             from[0],
//                             TO_EUCLID(renderer, from[1]),
//                             to[0],
//                             TO_EUCLID(renderer, to[1]));
//     }
// }


/*
void
Vector_Render(Renderer& renderer, Vec const& pos, Vec const& vel, Rotor2D const& avel)
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

    // Scale the lengths only. We don't want to scale the perpendicular part
    // of the cross.
    tail[0][0] *= s / a;
    tail[1][0] *= s / a;
    cross[0][0] *= s / a - 2.f; // draw the cross 2 pixels down from the tip.
    cross[1][0] *= s / a - 2.f;

    auto e1 = Vec { 1.0f, 0.0f, 0.0 };
    auto M  = Vec_Mul2D(e1, vel);

    SRT(tail, 1, M, pos);
    SRT(cross, 1, M, pos);

    SDL_SetRenderDrawColor(renderer.renderer,
                           0,
                           0,
                           0,
                           255);

    SDL_FRect base { pos.x - 1.f,
                     TO_EUCLID(renderer, pos.y - 1.f),
                     3.f,
                     3.f };

    SDL_RenderDrawRectF(renderer.renderer, &base);

    SDL_FRect area { pos.x,
                     TO_EUCLID(renderer, pos.y),
                     avel.s * 100.f,
                     avel.I * 100.f };

    SDL_RenderDrawRectF(renderer.renderer, &area);

    Renderer_DrawLines(tail, renderer);
    Renderer_DrawLines(cross, renderer);
}
*/