//
// Created by lukas on 10/12/25.
//
#include "game.h"

#include <stdio.h>
#include <stdlib.h>

GAME init_game() {
    GAME game;
    game.tilemap = init_tilemap();
    game.game_objects =  malloc(sizeof(GAME_OBJECT) * STARTING_COUNT_OF_GAME_OBJECTS);
    game.object_capacity = STARTING_COUNT_OF_GAME_OBJECTS;
    game.object_count = 0;
    game.player_money = 100;
    game.player_lives = 100;
    game.next_id = 0;
    add_game_object(&game,init_tower((Vector2){5,3},&game));
    add_game_object(&game,init_tower((Vector2){8,11},&game));
    add_game_object(&game,init_tower((Vector2){20,3},&game));
    add_game_object(&game,init_tower((Vector2){20,11},&game));


    return game;
}

GAME_OBJECT init_tower(const Vector2 position, const GAME *game) {
     return (GAME_OBJECT) {
         game->next_id,
         TOWER,
         position,
         (TOWER_DATA) {
             0,
             0,
             1000,
             0,
             4,
             4
         }
     };
}


void add_game_object(GAME *game, const GAME_OBJECT game_object) {
    if (game->object_count + 1 >= game->object_capacity) {
        add_object_size(game);
    }
    game->game_objects[game->object_count] = game_object;
    game->object_count++;
    game->next_id++;
}

void add_object_size(GAME* game) {
    int new_capacity = game->object_capacity * 2;
    if (new_capacity == 0) new_capacity = STARTING_COUNT_OF_GAME_OBJECTS;

    GAME_OBJECT* temp = realloc(game->game_objects, sizeof(GAME_OBJECT) * new_capacity);

    if (temp == NULL) {
        fprintf(stderr, "ERROR: Failed to reallocate memory for game objects.\n");
        exit(1);
    }

    game->game_objects = temp;
    game->object_capacity = new_capacity;
}

void start_game(GAME *game) {

    while (!WindowShouldClose())
    {

        const GRID_COORD grid_pos = screen_to_grid(GetMousePosition(),&game->tilemap);

        const int scaled_tile_size = get_tile_scale(&game->tilemap);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            printf("Player clicked on grid cell: (%d, %d)\n", grid_pos.x, grid_pos.y);
            upgrade_clicked_tower(game,grid_pos);


        }
        BeginDrawing();
        ClearBackground(BLACK);

        draw_tilemap(&game->tilemap);
        BeginDrawing();
        ClearBackground(BLACK);
        draw_tilemap(&game->tilemap);

        if (grid_pos.x >= 0 && grid_pos.x < game->tilemap.map_width &&
            grid_pos.y >= 0 && grid_pos.y < game->tilemap.map_height) {

            DrawRectangleLines(
                grid_pos.x * scaled_tile_size,
                grid_pos.y * scaled_tile_size,
                scaled_tile_size,
                scaled_tile_size,
                YELLOW
            );
        }
        DrawFPS(10, 10);

        EndDrawing();
    }
    unload_game(game);
}

void unload_game(GAME *game) {
    unload_tilemap(&game->tilemap);
    free(game->game_objects);
    game->game_objects = nullptr;
    printf("GAME: was unloaded\n");
}

GRID_COORD screen_to_grid(Vector2 screen_pos, const TILE_MAP* tilemap) {
    const int scaled_tile_size = get_tile_scale(tilemap);

    GRID_COORD grid_pos;
    grid_pos.x = (int)(screen_pos.x / (float)scaled_tile_size);
    grid_pos.y = (int)(screen_pos.y / (float)scaled_tile_size);

    return grid_pos;
}

/**
 * IMPORTANT: The caller is responsible for freeing the memory allocated for 'out_objects'
 * @return The number of objects found, or -1 on memory allocation failure.
 */
int get_game_objects_of_type(const GAME *game, const OBJECT_TYPE type, GAME_OBJECT **out_objects) {
    int count = 0;
    for (int i = 0; i < game->object_count;i++) {
        if (game->game_objects[i].type == type) {
            count++;
        }
    }
    if (count == 0) {
        *out_objects = nullptr;
        return 0;
    }

    *out_objects = malloc(sizeof(GAME_OBJECT) * count);

    if (*out_objects == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate memory for game objects array.\n");
        return -1;
    }

    int current_index = 0;
    for (int i = 0; i < game->object_count; i++) {
        if (game->game_objects[i].type == type) {
            (*out_objects)[current_index] = game->game_objects[i];
            current_index++;
        }
    }

    return count;
}

bool upgrade_clicked_tower(GAME *game, const GRID_COORD grid_coord) {

    if (game->player_money < 100) {
        return 0;
    }
    int clicked_tower_id = -1;
    for (int i = 0; i < game->object_count; i++) {
        if (game->game_objects[i].type != TOWER) {
            continue;
        }
        GAME_OBJECT* tower = &game->game_objects[i];
        const int tower_grid_x = (int)tower->position.x;
        const int tower_grid_y = (int)tower->position.y;
        const int tower_width  = tower->data.tower.width;
        const int tower_height = tower->data.tower.height;

        const bool is_inside = grid_coord.x >= tower_grid_x &&
                               grid_coord.x < tower_grid_x + tower_width &&
                               grid_coord.y >= tower_grid_y &&
                               grid_coord.y < tower_grid_y + tower_height;
        if (is_inside) {
            tower->data.tower.level++;
            game->player_money -= 100;
            clicked_tower_id = tower->id;
            break;
        }

    }
    return clicked_tower_id != -1;
}


