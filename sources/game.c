//
// Created by lukas on 10/12/25.
//
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GAME init_game() {
    GAME game;
    game.tilemap = init_tilemap();
    game.game_objects =  malloc(sizeof(GAME_OBJECT) * STARTING_COUNT_OF_GAME_OBJECTS);
    game.object_capacity = STARTING_COUNT_OF_GAME_OBJECTS;
    game.object_count = 0;
    game.player_money = 1000;
    game.player_lives = 100;
    game.next_id = 0;

    game.assets.towers = LoadTexture(ASSETS_PATH "images/towers.png");

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
             -1,
             4,
             4,
             LEVEL_0
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
        draw_game_objects(game);

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
    UnloadTexture(game->assets.towers);
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

    if (*out_objects == nullptr) {
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

        if (tower->data.tower.level != LEVEL_0) {
            continue;
        }

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

TowerSpriteInfo get_tower_sprite_info(const TOWER_LEVEL level) {
    static const int level_0_sprites[] = {
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15
    };
    static const int level_1_sprites[] = {
        16, 17, 18, 19,
        20, 21, 22, 23,
        24, 25, 26, 27,
        28, 29, 30, 31
    };
    TowerSpriteInfo info = { .sprites = nullptr, .count = 0, .width = 0, .height = 0 };

    switch (level) {
        case LEVEL_0:
            info.sprites = level_0_sprites;
            info.count = sizeof(level_0_sprites) / sizeof(level_0_sprites[0]);
            info.width = 4;
            info.height = 4;
            break;
        case LEVEL_1:
            info.sprites = level_1_sprites;
            info.count = sizeof(level_1_sprites) / sizeof(level_1_sprites[0]);
            info.width = 4;
            info.height = 4;
            break;
    }
    return info;
}

void draw_game_objects(const GAME* game) {
    for (int i = 0; i < game->object_count; i++) {
        const GAME_OBJECT* obj = &game->game_objects[i];

        if (obj->type == TOWER) {
            const TowerSpriteInfo info = get_tower_sprite_info(obj->data.tower.level);

            if (info.sprites == NULL) {
                continue;
            }

            for (int y = 0; y < info.height; y++) {
                for (int x = 0; x < info.width; x++) {
                    const int sprite = info.sprites[info.width * y + x];
                    draw_texture(&game->tilemap,game->assets.towers, sprite, (int)obj->position.x + x, (int)obj->position.y + y);
                }
            }

        }
    }
}
