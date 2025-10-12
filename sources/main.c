#include "raylib.h"
#include "tilemap.h"
#include <stdio.h>

#define SCREEN_WIDTH (1024)
#define SCREEN_HEIGHT (576)

#define WINDOW_TITLE "Tower Defense"


int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);

    // Initialize the tilemap
    Tilemap map = InitTilemap();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawTilemap(&map);

        DrawFPS(10, 10);

        EndDrawing();
    }

    UnloadTilemap(&map);
    CloseWindow();

    return 0;
}
