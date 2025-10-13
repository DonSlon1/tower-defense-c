#include "game.h"
#include "raylib.h"
#include "tilemap.h"

#define SCREEN_WIDTH (1024)
#define SCREEN_HEIGHT (576)

#define WINDOW_TITLE "Tower Defense"


int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);
    SetWindowIcon(ASSETS_PATH "images/towers.png");
    SetMouseCursor(ASSETS_PATH "cursor/Middle Ages--cursor--SweezyCursors.png");
    SetMousePointer(ASSETS_PATH "cursor/Middle Ages--pointer--SweezyCursors.png");

    GAME game = init_game();

    start_game(&game);

    CloseWindow();

    return 0;
}
