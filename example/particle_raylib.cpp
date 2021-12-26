#include "Base/typedefs.h"
#include "GeometricAlgebra/geometric_algebra.h"
#include "Particles/particle.h"
#include "SmallLib/smallmath.h"
#include "extras/raygui.h"
#include "raylib.h"
#include <cassert>
#include <stdio.h>
#include <stdlib.h>

extern void
DrawRotatedCube(Vector3 position, Vector3 dimensions, float* mat4x4, Color color);
extern void
DrawTerrain();

struct Player
{
    union
    {
        Vector2 pos;
        struct
        {
            float x, y;
        };
    };
    float w, h;
};


struct Window
{
    char const* title { "raylib [core] example - 2d camera" };
    int         w { 800 };
    int         h { 600 };
    float       half_w { w / 2.0f };
    float       half_h { h / 2.0f };

    Camera2D camera;
};

enum class Viewport_Size
{
    Full,
    Windowed
};

struct Viewport
{
    Viewport_Size size { Viewport_Size::Windowed };

    int   w;
    int   h;
    float half_w;
    float half_h;

    // Min and is set to the smaller of w or h.
    // This is useful when converting from world to screen space as we want
    // 1 world space to a) be square and b) fit entirely within the viewport.
    int min;
    int half_min;

    Camera          camera;
    RenderTexture2D texture;
};


struct GameStruct
{
    int      framerate { 60 };
    Window   window;
    Viewport viewport;

    Particle particle;
    Emitter  emitter;
};


void
Print(char const* text, Vector2 const& v)
{
    printf("%s %f %f\n", text, v.x, v.y);
}

void
Print(char const* text, float x)
{
    printf("%s %f\n", text, x);
}


void
Viewport_Scale(Viewport& viewport, Window const& window, float percent)
{
    assert(percent > 0 && percent <= 1.0);
    viewport.w      = RoundFloatToInt(window.w * percent);
    viewport.h      = RoundFloatToInt(window.h * percent);
    viewport.half_w = viewport.w / 2;
    viewport.half_h = viewport.h / 2;

    viewport.min      = Min(viewport.w, viewport.h);
    viewport.half_min = viewport.min / 2;

    viewport.texture           = LoadRenderTexture(viewport.w, viewport.h);
    viewport.camera.up         = { 0.0f, 1.0f, 0.0f };
    viewport.camera.fovy       = 45.0f;
    viewport.camera.projection = CAMERA_PERSPECTIVE;
}

float
Viewport_GetScale(Viewport& viewport)
{
    if (viewport.size == Viewport_Size::Full)
    {
        return 1.0f;
    }
    else
    {
        return 0.5;
    }
}

void
Viewport_ChangeSize(Viewport& viewport, Window const& window)
{
    if (viewport.size == Viewport_Size::Full)
    {
        viewport.size = Viewport_Size::Windowed;
    }
    else
    {
        viewport.size = Viewport_Size::Full;
    }
    float percent = Viewport_GetScale(viewport);
    Viewport_Scale(viewport, window, percent);
}


void
Viewport_Init(Viewport& viewport, Window const& window)
{
    viewport.camera = { 0 };
    auto percent    = Viewport_GetScale(viewport);
    Viewport_Scale(viewport, window, percent);

    // NOTE(DW): We don't want to adjust the zoom or rotation if we change
    // the viewport size.
    viewport.camera.position = { 10.0f, 10.0f, 10.0f };
    viewport.camera.target   = { 0.0f, 0.0f, 0.0f };
}

struct Attributes
{
    int health;
    int attack;
    int speed; // 0 - slow, 5 - normal, 10 - fast
    int timer; // 5 seconds when normal (5*60fps = 300)
};

struct Battle
{
    std::vector<Attributes> players;
    std::vector<Attributes> enemies;
};


void
Battle_Intergrate(Battle& battle, float time_secs)
{

    for (auto& p : battle.players)
    {
        p.timer += p.speed;
    }
    for (auto& e : battle.enemies)
    {
        e.timer += e.speed;
    }
}

void
UpdateGame(GameStruct& game)
{
    auto& particle = game.particle;
    Particle_Integrate(particle, 1.0f / game.framerate);
    Emitter_Integrate(game.emitter, 1.0f / game.framerate);
}


void
UpdateKeyboard(GameStruct& game)
{
    if (IsKeyDown('Z'))
    {
        game.viewport.camera.target = (Vector3) { 0.0f, 0.0f, 0.0f };
    }

    if (IsKeyDown('V'))
    {

        Viewport_ChangeSize(game.viewport, game.window);
    }
}

struct Grid
{
    int x, y;
    int rows, cols;
    int spacing;

    Rectangle items[100];
};


void
Grid_Init(Grid& grid, int x, int y, int rows, int cols, int spacing)
{
    assert(rows * cols < 100);
    grid.x       = x;
    grid.y       = y;
    grid.rows    = rows;
    grid.cols    = cols;
    grid.spacing = spacing;
}


void
Grid_ConfigureRows(Grid& grid, float height)
{
    int c = 0;
    int r = 0;
    for (r = 0; r < grid.rows; ++r)
    {
        int y = grid.y + r * height + r * grid.spacing;
        for (c = 0; c < grid.cols; ++c)
        {
            auto& item  = grid.items[grid.cols * r + c];
            item.y      = y;
            item.height = height;
        }
    }
}


void
Grid_ConfigureColumns(Grid& grid, float width)
{
    int c = 0;
    int r = 0;
    for (r = 0; r < grid.rows; ++r)
    {
        for (c = 0; c < grid.cols; ++c)
        {
            auto& item = grid.items[grid.cols * r + c];
            item.x     = grid.x + c * width + c * grid.spacing;
            item.width = width;
        }
    }
}


void
Grid_ConfigureColumn(Grid& grid, int col, float width)
{
    assert(col < grid.cols);
    int c = 0;
    int r = 0;
    for (r = 0; r < grid.rows; ++r)
    {
        int offset = grid.x;
        for (c = 0; c < col; ++c)
        {
            auto& item = grid.items[grid.cols * r + c];
            offset += item.width + grid.spacing;
        }

        auto& item = grid.items[grid.cols * r + col];
        item.x     = offset;
        item.width = width;

        offset += width + grid.spacing;

        for (c = col + 1; c < grid.cols; ++c)
        {
            auto& item = grid.items[grid.cols * r + c];
            item.x     = offset;
            offset += item.width + grid.spacing;
        }
    }
}


Rectangle
Grid_GetRect(Grid const& grid, int row, int col)
{
    assert(row >= 0);
    assert(row < grid.rows);
    assert(col >= 0);
    assert(col < grid.cols);
    return grid.items[grid.cols * row + col];
}


void
Print(char const* text, Rectangle const& r)
{
    printf("%s x:%f y:%f w:%f h:%f\n", text, r.x, r.y, r.width, r.height);
}


void
Gui_WidgetFromProperty(Property& property, Rectangle const& rect)
{
    switch (property.tag)
    {
    case Property_Tag::Float:
    {
        auto& floatp = property.prop_float;
        if (property.read_only)
        {
            assert(false);
        }
        else
        {
            *floatp.ptr = GuiSlider(rect, "", "", *floatp.ptr, floatp.min, floatp.max);
        }
        break;
    }
    case Property_Tag::SizeT:
    {
        auto& sizep = property.prop_sizet;
        if (property.read_only)
        {
            GuiProgressBar(rect, "", "", sizep.value, sizep.min, sizep.max);
        }
        else
        {
            assert(false);
        }
        break;
    }
    }
}

int
main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    GameStruct game;

    InitWindow(game.window.w, game.window.h, game.window.title);

    Viewport_Init(game.viewport, game.window);

    // Define the camera to look into our 3d world
    auto& camera    = game.viewport.camera;
    camera.position = { 15.0f, 15.0f, 25.0f };

    auto cube_position   = Vector3 { 0.0f, 0.0f, 0.0f };
    auto cube_dimensions = Vector3 { 2.0f * 0.707f, 2.0f * 0.707f, 2.0f * 0.707f };

    SetCameraMode(camera, CAMERA_FREE); // Set a free camera mode

    // Set our game to run at 60 frames-per-second
    SetTargetFPS(game.framerate);

    //--------------------------------------------------------------------------------------
    Particle_Init(game.particle);
    Emitter_Init(game.emitter);

    auto theta_e1e2 = game.particle.omega.x;
    auto theta_e1e3 = game.particle.omega.y;
    auto theta_e2e3 = game.particle.omega.z;

    int  x_offset = game.window.w - (50 + 150 + 25 + 25) - (4 * 4);
    Grid grid;
    int  Grid_Rows    = 7;
    int  Grid_Cols    = 5;
    int  Grid_Spacing = 4;
    Grid_Init(grid, x_offset, 4, Grid_Rows, Grid_Cols, Grid_Spacing);
    Grid_ConfigureRows(grid, 20);
    Grid_ConfigureColumns(grid, 50);
    // Grid_ConfigureColumn(grid, 1, 100);
    Grid_ConfigureColumn(grid, 1, 150);
    Grid_ConfigureColumn(grid, 2, 25);
    Grid_ConfigureColumn(grid, 3, 25);


    int  val     = 0;
    bool pressed = false;
    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);
        UpdateKeyboard(game);
        UpdateGame(game);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(WHITE);

        BeginTextureMode(game.viewport.texture);
        ClearBackground(SKYBLUE);
        BeginMode3D(camera);

        DrawGrid(100, 1.0f);
        // DrawTerrain();

#if 0
        cube_position = {
            game.particle.pos.x,
            game.particle.pos.y,
            game.particle.pos.z,
        };
        DrawRotatedCube(cube_position,
                        cube_dimensions,
                        &game.particle.rot_mat[0],
                        RED);
#else
        for (auto& particle : game.emitter.particles)
        {
            cube_position = {
                particle.pos.x,
                particle.pos.y,
                particle.pos.z,
            };
            DrawRotatedCube(cube_position,
                            cube_dimensions,
                            &particle.rot_mat[0],
                            RED);
        }
#endif


        EndMode3D();
        EndTextureMode();

        DrawTextureRec(game.viewport.texture.texture,
                       Rectangle { 0.0f, 0.0f, (float)game.viewport.w, -(float)game.viewport.h },
                       Vector2 { 0, 0 },
                       WHITE);

        // DrawRectangle(11, 10, 320, 133, Fade(SKYBLUE, 0.5f));
        // DrawRectangleLines(11, 10, 320, 133, BLUE);

        // DrawText("Free camera default controls:", 21, 20, 10, BLACK);
        // DrawText("- Mouse Wheel to Zoom in-out", 41, 40, 10, DARKGRAY);
        // DrawText("- Mouse Wheel Pressed to Pan", 41, 60, 10, DARKGRAY);
        // DrawText("- Alt + Mouse Wheel Pressed to Rotate", 41, 80, 10, DARKGRAY);
        // DrawText("- Alt + Ctrl + Mouse Wheel Pressed for Smooth Zoom", 41, 100, 10, DARKGRAY);
        // DrawText("- Z to zoom to (1, 0, 0)", 40, 120, 10, DARKGRAY);

        int       row = 0;
        int       col = 0;
        Rectangle r;
        {
            col = 0;

            ++col;
            r            = Grid_GetRect(grid, row, col++);
            auto pressed = GuiButton(r, GuiIconText(RICON_UNDO, "reset"));
            if (pressed)
            {
                theta_e1e2          = 0;
                theta_e1e3          = 0;
                theta_e2e3          = 0;
                game.particle.theta = { 0.0, 0.0, 0.0 };
                game.particle.omega = { 0.0, 0.0, 0.0 };
            }

            row += 1;
        }
        {
            col = 0;

            GuiLabel(Grid_GetRect(grid, row, col++), "e1e2");
            // if (GuiValueBox(Grid_GetRect(grid, row, col++), "Hello", &val, RoundFloatToInt(-M_PI * RAD2DEG), RoundFloatToInt(M_PI * RAD2DEG), pressed))
            // {
            //     pressed = !pressed;
            // }
            theta_e1e2 = GuiSliderBar(Grid_GetRect(grid, row, col++), "", "", theta_e1e2, -2 * M_PI, 2 * M_PI);

            ++col;
            r            = Grid_GetRect(grid, row, col++);
            auto pressed = GuiButton(r, GuiIconText(RICON_UNDO, ""));
            if (pressed)
            {
                theta_e1e2 = 0;
            }

            row += 1;
        }
        {
            col = 0;

            r = Grid_GetRect(grid, row, col++);
            GuiLabel(r, "e1e3");

            r          = Grid_GetRect(grid, row, col++);
            theta_e1e3 = GuiSliderBar(r, "-pi", "pi", theta_e1e3, -2 * M_PI, 2 * M_PI);
            ++col;
            r            = Grid_GetRect(grid, row, col++);
            auto pressed = GuiButton(r, GuiIconText(RICON_UNDO, ""));
            if (pressed)
            {
                theta_e1e3 = 0;
            }

            row += 1;
        }
        {
            col = 0;

            r = Grid_GetRect(grid, row, col++);
            GuiLabel(r, "e2e3");

            r          = Grid_GetRect(grid, row, col++);
            theta_e2e3 = GuiSliderBar(r, "-pi", "pi", theta_e2e3, -2 * M_PI, 2 * M_PI);

            ++col;
            r            = Grid_GetRect(grid, row, col++);
            auto pressed = GuiButton(r, GuiIconText(RICON_UNDO, ""));
            if (pressed)
            {
                theta_e2e3 = 0;
            }

            row += 1;
        }
        {
            col = 0;

            auto emitter_props = Emitter_Properties(game.emitter);
            assert(emitter_props.size() > 0);

            r = Grid_GetRect(grid, row, col++);
            GuiLabel(r, emitter_props[0].name);

            r = Grid_GetRect(grid, row, col++);
            Gui_WidgetFromProperty(emitter_props[0], r);
            row += 1;
        }
        {
            col = 0;

            auto emitter_props = Emitter_Properties(game.emitter);
            assert(emitter_props.size() > 0);

            r = Grid_GetRect(grid, row, col++);
            GuiLabel(r, emitter_props[1].name);

            r = Grid_GetRect(grid, row, col++);
            Gui_WidgetFromProperty(emitter_props[1], r);

            row += 1;
        }

        game.particle.omega = { theta_e1e2, theta_e1e3, theta_e2e3 };

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}