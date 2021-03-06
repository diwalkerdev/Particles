#include "Base/application.h"
#include "Base/containers/array.h"
#include "Base/debug_services.h"
#include "Base/dllexports.h"
#include "Base/typedefs.h"
#include "Easing/engine.h"
#include "Layout/Layouts/horizontal.h"
#include "Layout/Layouts/span.h"
#include "Layout/Layouts/vertical.h"
#include "Layout/Widgets/button.h"
#include "Layout/Widgets/label.h"
#include "Layout/layout.h"
#include "Layout/mainloop.h"
#include "Layout/mousearea.h"
#include "Layout/render.h"
#include "Layout/resize.h"
#include "Layout/scene.h"
#include "Layout/textrenderer.h"
#include "Layout/tree.h"
#include "Layout/treeiter.h"
#include "Layout/treeutil.h"
#include "Particles/particle.h"

#if defined(_MSC_VER)
#include <SDL.h>
#include <SDL_scancode.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_scancode.h>
#endif

#include <cassert>
#include <stdio.h>


// TODO: The rending is currently limited by the polling.
// We need to poll the timers, at a rate of whatever is quickest.
compile_float(POLLS_PER_SEC, 30.0);
compile_float(RENDERS_PER_SEC, 30.0);
compile_float(SIMS_PER_SEC, 30.0);


struct GameStruct
{
    Window       window;
    EventManager events;

    Emitter  emitter;
    Particle particle;

    struct UI
    {
        Scene<30> scene;
    } ui;
};


void
EventManager_Init(EventManager& event_manager)
{
    EventManager_AddKeyBinding(event_manager, SDL_SCANCODE_M, EVENT_NO_EVENT);
}


void
GameLoop_Render(GameStruct& game_struct)
{
    TIME_BLOCK;

    SDL_Renderer* sdl_renderer = game_struct.window.renderer.renderer;

    Window_Clear(game_struct.window);

    Layout_Resize2(game_struct.ui.scene.tree,
                   game_struct.window.w,
                   game_struct.window.h);

    //Tree_Print(game_struct.ui.scene.tree);

    Layout_Render(sdl_renderer,
                  game_struct.ui.scene.tree);

    game_struct.ui.scene.ProcessMouseEvents(game_struct.events);
    game_struct.ui.scene.RenderWidgets(sdl_renderer);

    Emitter_Render(game_struct.emitter, sdl_renderer);
    Particle_Render(game_struct.particle, game_struct.window.renderer);
    static float rad_per_sec = (90.f / RENDERS_PER_SEC) * M_PI / 180.f;

    static Vec vel { 100.f, 0, 0 };
    static Vec u { cosf(rad_per_sec), sinf(rad_per_sec), 0.f };

    static Rotor2D avel = Vec_Mul2D(vel, u);

    vel = Vec_Rotate(vel, avel);

    Vector_Render(game_struct.window.renderer,
                  game_struct.particle.pos,
                  game_struct.particle.vel,
                  avel);

    Window_PresentRenderer(game_struct.window);
}


void
GameLoop_Simulate(GameStruct& game_struct)
{
    TIME_BLOCK;

    float ms = 1000.f / SIMS_PER_SEC;
    Easing_EngineIntegrate(ms);

    Emitter_Integrate(game_struct.emitter, ms);
    Particle_Integrate(game_struct.particle, ms);
}


void
GameLoop_HandleInputs(GameStruct& game_struct)
{
    auto* m_bind = EventManager_FindKeyBinding(game_struct.events, SDL_SCANCODE_M);

    if (m_bind->state == KEY_PRESSED)
    {
        printf("m\n");
    }
}


void
InitUI(GameStruct& game_struct)
{
    auto& scene = game_struct.ui.scene;
    Easing_Free(global_bfv);

    auto root_index = scene.Init<Layout_Vertical>(SizePolicy::Expand,
                                                  SizePolicy::Expand);

    auto& layout = scene.LayoutAt(root_index);
    Layout_ToggleVisibility(layout);

    Tree_Print(game_struct.ui.scene.tree);
}


int
main(int argc, char** argv)
{
    GameStruct game_struct;

    GameLoop_Init<GameStruct>(game_struct,
                              RENDERS_PER_SEC,
                              SIMS_PER_SEC,
                              argc,
                              argv);

    SDL_SetRenderDrawBlendMode(game_struct.window.renderer.renderer,
                               SDL_BLENDMODE_BLEND);
    InitUI(game_struct);
    Emitter_Init(game_struct.emitter);
    Particle_Init(game_struct.particle);

    GameLoop_Main<GameStruct>(game_struct,
                              POLLS_PER_SEC,
                              argc,
                              argv);

    GameLoop_Free<GameStruct>(game_struct);
}
