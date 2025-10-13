#include "game.h"
#include "raylib.h"

#define SCREEN_WIDTH (1024)
#define SCREEN_HEIGHT (576)

#define WINDOW_TITLE "Tower Defense"


int main(void)
{
    init_window(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    set_target_fps(60);
    set_window_icon(ASSETS_PATH "images/towers.png");
    set_mouse_cursor(ASSETS_PATH "cursor/Middle Ages--cursor--SweezyCursors.png");
    set_mouse_pointer(ASSETS_PATH "cursor/Middle Ages--pointer--SweezyCursors.png");

    game game = init_game();

    start_game(&game);

    close_window();

    return 0;
}
