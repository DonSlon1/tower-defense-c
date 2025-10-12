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
    if (game.game_objects == nullptr) {
        fprintf(stderr, "ERROR: Failed to allocate memory for game objects\n");
        unload_tilemap(&game.tilemap);
        exit(1);
    }
    game.object_capacity = STARTING_COUNT_OF_GAME_OBJECTS;
    game.object_count = 0;
    game.player_money = STARTING_AMOUNT_OF_MONEY;
    game.player_lives = STARTING_AMOUNT_OF_LIVES;
    game.next_id = 0;

    game.assets.towers = LoadTexture(ASSETS_PATH "images/towers.png");
    if (game.assets.towers.id == 0) {
        fprintf(stderr, "ERROR: Failed to load towers texture\n");
        free(game.game_objects);
        unload_tilemap(&game.tilemap);
        exit(1);
    }

    add_game_object(&game,init_tower((Vector2){5,3}));
    add_game_object(&game,init_tower((Vector2){8,11}));
    add_game_object(&game,init_tower((Vector2){20,3}));
    add_game_object(&game,init_tower((Vector2){20,11}));


    return game;
}

GAME_OBJECT init_tower(const Vector2 position) {
  return (GAME_OBJECT) {
      .type = TOWER,
      .position = position,
      .data.tower = {
          .damage = TOWER_LEVEL_0_DAMAGE,
          .range = TOWER_LEVEL_0_RANGE,
          .fire_cooldown = TOWER_LEVEL_0_FIRE_COOLDOWN,
          .target_id = -1,
          .width = TOWER_LEVEL_0_WIDTH,
          .height = TOWER_LEVEL_0_HEIGHT,
          .upgrade_cost = TOWER_LEVEL_0_UPGRADE_COST,
          .level = LEVEL_0
      }
  };
}


void add_game_object(GAME *game, GAME_OBJECT game_object) {
    if (game->object_count == game->object_capacity) {
        grow_object_capacity(game);
    }

    game_object.id = game->next_id;
    game->game_objects[game->object_count] = game_object;
    game->object_count++;
    game->next_id++;
}

void grow_object_capacity(GAME* game) {
    size_t new_capacity = game->object_capacity * 2;
    if (new_capacity == 0) new_capacity = STARTING_COUNT_OF_GAME_OBJECTS;

    if (new_capacity < game->object_capacity) {
        fprintf(stderr, "ERROR: Capacity overflow when resizing game objects array.\n");
        exit(1);
    }

    GAME_OBJECT* temp = realloc(game->game_objects, sizeof(GAME_OBJECT) * new_capacity);

    if (temp == nullptr) {
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
            const UPGRADE_RESULT result = upgrade_clicked_tower(game,grid_pos);

            switch (result) {
                case UPGRADE_SUCCESS:
                    printf("Tower upgraded successfully!\n");
                    break;
                case UPGRADE_INSUFFICIENT_FUNDS:
                    printf("Insufficient funds for upgrade\n");
                    break;
                case UPGRADE_MAX_LEVEL:
                    printf("Tower is already at max level\n");
                    break;
                case UPGRADE_NOT_FOUND:
                    break;
            }
        }
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

GRID_COORD screen_to_grid(const Vector2 screen_pos, const TILE_MAP* tilemap) {
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
    for (size_t i = 0; i < game->object_count;i++) {
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
    for (size_t i = 0; i < game->object_count; i++) {
        if (game->game_objects[i].type == type) {
            (*out_objects)[current_index] = game->game_objects[i];
            current_index++;
        }
    }

    return count;
}

UPGRADE_RESULT upgrade_clicked_tower(GAME *game, const GRID_COORD grid_coord) {
    for (size_t i = 0; i < game->object_count; i++) {
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
            if (tower->data.tower.level >= LEVEL_1) {
                return UPGRADE_MAX_LEVEL;
            }

            if (game->player_money < tower->data.tower.upgrade_cost) {
                return UPGRADE_INSUFFICIENT_FUNDS;
            }

            tower->data.tower.level++;
            game->player_money -= tower->data.tower.upgrade_cost;
            return UPGRADE_SUCCESS;
        }
    }
    return UPGRADE_NOT_FOUND;
}

TowerSpriteInfo get_tower_sprites(const TOWER_LEVEL level) {
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
        default:
            fprintf(stderr, "WARNING: Failed to find sprite for tower level\n");
    }
    return info;
}

void draw_game_objects(const GAME* game) {
    for (size_t i = 0; i < game->object_count; i++) {
        const GAME_OBJECT* obj = &game->game_objects[i];

        if (obj->type == TOWER) {
            const TowerSpriteInfo info = get_tower_sprites(obj->data.tower.level);

            if (info.sprites == nullptr) {
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
