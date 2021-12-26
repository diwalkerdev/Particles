#include "SmallLib/smallalg.h"
#include "raylib.h" // Declares module functions

// Check if config flags have been externally provided on compilation line
// #if !defined(EXTERNAL_CONFIG_FLAGS)
// #include "config.h" // Defines module configuration flags
// #endif

#include "raymath.h" // Required for: Vector3, Quaternion and Matrix functionality
#include "rlgl.h" // OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2


//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// #ifndef MAX_MATERIAL_MAPS
// #define MAX_MATERIAL_MAPS 12 // Maximum number of maps supported
// #endif
// #ifndef MAX_MESH_VERTEX_BUFFERS
// #define MAX_MESH_VERTEX_BUFFERS 7 // Maximum vertex buffers (VBO) per mesh
// #endif

#include <stdio.h>

static void
Print(char const* text, Matrix const& mat)
{
    printf("%s\n", text);
    printf("%f %f %f %f\n", mat.m0, mat.m4, mat.m8, mat.m12);
    printf("%f %f %f %f\n", mat.m1, mat.m5, mat.m9, mat.m13);
    printf("%f %f %f %f\n", mat.m2, mat.m6, mat.m10, mat.m14);
    printf("%f %f %f %f\n", mat.m3, mat.m7, mat.m11, mat.m15);
}

static void
Print(char const* text, float* mat)
{
    printf("%s\n", text);
    for (int i = 0; i < 4; ++i)
    {
        for (int k = 0; k < 4; ++k)
        {
            printf("%f ", mat[4 * i + k]);
        }
        printf("\n");
    }
}

void
DrawRotatedCube(Vector3 position, Vector3 dimensions, float* mat4x4, Color color)
{
    float x      = 0.0f;
    float y      = 0.0f;
    float z      = 0.0f;
    auto  width  = dimensions.x;
    auto  height = dimensions.y;
    auto  length = dimensions.z;

    rlCheckRenderBatchLimit(36);
    rlMatrixMode(RL_MODELVIEW);
    rlPushMatrix();
    // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)

    float t[16] = {
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, position.x, position.y, position.z, 1
    };

    rlMultMatrixf(&mat4x4[0]);
    rlMultMatrixf(&t[0]);

    rlBegin(RL_TRIANGLES);
    rlColor4ub(0, 0, 0, color.a);

    // Front face
    rlVertex3f(x - width / 2, y - height / 2, z + length / 2); // Bottom Left
    rlVertex3f(x + width / 2, y - height / 2, z + length / 2); // Bottom Right
    rlVertex3f(x - width / 2, y + height / 2, z + length / 2); // Top Left

    rlVertex3f(x + width / 2, y + height / 2, z + length / 2); // Top Right
    rlVertex3f(x - width / 2, y + height / 2, z + length / 2); // Top Left
    rlVertex3f(x + width / 2, y - height / 2, z + length / 2); // Bottom Right

    rlColor4ub(color.r, color.g, color.b, color.a);
    // Back face
    rlVertex3f(x - width / 2, y - height / 2, z - length / 2); // Bottom Left
    rlVertex3f(x - width / 2, y + height / 2, z - length / 2); // Top Left
    rlVertex3f(x + width / 2, y - height / 2, z - length / 2); // Bottom Right

    rlVertex3f(x + width / 2, y + height / 2, z - length / 2); // Top Right
    rlVertex3f(x + width / 2, y - height / 2, z - length / 2); // Bottom Right
    rlVertex3f(x - width / 2, y + height / 2, z - length / 2); // Top Left

    // Top face
    rlVertex3f(x - width / 2, y + height / 2, z - length / 2); // Top Left
    rlVertex3f(x - width / 2, y + height / 2, z + length / 2); // Bottom Left
    rlVertex3f(x + width / 2, y + height / 2, z + length / 2); // Bottom Right

    rlVertex3f(x + width / 2, y + height / 2, z - length / 2); // Top Right
    rlVertex3f(x - width / 2, y + height / 2, z - length / 2); // Top Left
    rlVertex3f(x + width / 2, y + height / 2, z + length / 2); // Bottom Right

    // Bottom face
    rlVertex3f(x - width / 2, y - height / 2, z - length / 2); // Top Left
    rlVertex3f(x + width / 2, y - height / 2, z + length / 2); // Bottom Right
    rlVertex3f(x - width / 2, y - height / 2, z + length / 2); // Bottom Left

    rlVertex3f(x + width / 2, y - height / 2, z - length / 2); // Top Right
    rlVertex3f(x + width / 2, y - height / 2, z + length / 2); // Bottom Right
    rlVertex3f(x - width / 2, y - height / 2, z - length / 2); // Top Left

    // Right face
    rlVertex3f(x + width / 2, y - height / 2, z - length / 2); // Bottom Right
    rlVertex3f(x + width / 2, y + height / 2, z - length / 2); // Top Right
    rlVertex3f(x + width / 2, y + height / 2, z + length / 2); // Top Left

    rlVertex3f(x + width / 2, y - height / 2, z + length / 2); // Bottom Left
    rlVertex3f(x + width / 2, y - height / 2, z - length / 2); // Bottom Right
    rlVertex3f(x + width / 2, y + height / 2, z + length / 2); // Top Left

    // Left face
    rlVertex3f(x - width / 2, y - height / 2, z - length / 2); // Bottom Right
    rlVertex3f(x - width / 2, y + height / 2, z + length / 2); // Top Left
    rlVertex3f(x - width / 2, y + height / 2, z - length / 2); // Top Right

    rlVertex3f(x - width / 2, y - height / 2, z + length / 2); // Bottom Left
    rlVertex3f(x - width / 2, y + height / 2, z + length / 2); // Top Left
    rlVertex3f(x - width / 2, y - height / 2, z - length / 2); // Bottom Right
    rlEnd();
    rlPopMatrix();
}

void
DrawTerrain()
{
    auto width       = 10.0f;
    auto height      = 0.0f;
    auto length      = 10.0f;
    auto half_width  = width / 2.0f;
    auto half_height = height / 2.0f;
    auto half_length = length / 2.0f;

    float x, y, z;
    y = 0;

    int const n_squares   = 4;
    int const n_verticies = n_squares * n_squares * 6;

    float x_offset = width / 2.0f - (n_squares * width / 2.0f);
    float y_offset = 0;
    float z_offset = length / 2.0f - (n_squares * length / 2.0f);
    float t[16]    = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x_offset, y_offset, z_offset, 1 };

    rlCheckRenderBatchLimit(n_verticies);
    rlMatrixMode(RL_MODELVIEW);

    rlPushMatrix();
    rlBegin(RL_TRIANGLES);

    rlMultMatrixf(&t[0]);

    for (int i = 0; i < n_squares; ++i)
    {
        for (int k = 0; k < n_squares; ++k)
        {
            rlColor4ub(200 + 10 * i,
                       200 + 10 * k,
                       200,
                       255);

            x = k * width;
            z = i * length;
            rlVertex3f(x - width / 2, y + height / 2, z - length / 2); // Top Left
            rlVertex3f(x - width / 2, y + height / 2, z + length / 2); // Bottom Left
            rlVertex3f(x + width / 2, y + height / 2, z + length / 2); // Bottom Right

            rlVertex3f(x + width / 2, y + height / 2, z - length / 2); // Top Right
            rlVertex3f(x - width / 2, y + height / 2, z - length / 2); // Top Left
            rlVertex3f(x + width / 2, y + height / 2, z + length / 2); // Bottom Right
        }
    }


    rlEnd();
    rlPopMatrix();

    DrawCylinder({ -5, 0, +5 }, 0.0f, 2.0f, 5.0f, 6, { 50, 200, 50, 255 });
    DrawCylinder({ -5, 0, -5 }, 0.0f, 2.0f, 5.0f, 6, { 50, 200, 50, 255 });
    DrawCylinder({ +5, 0, +5 }, 0.0f, 2.0f, 5.0f, 6, { 200, 50, 50, 255 });
    DrawCylinder({ +5, 0, -5 }, 0.0f, 2.0f, 5.0f, 6, { 200, 50, 50, 255 });
}