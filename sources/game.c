//
// Created by lukas on 10/12/25.
//
#include "game.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Path 0: Top path (row 2) -> right -> down -> right to exit
Vector2 path_0_waypoints[] = {
    (Vector2) {0, 2},    // Start at top-left of path
    (Vector2) {19, 2},   // Move right to column 19
    (Vector2) {19, 7},   // Move down to row 7
    (Vector2) {24, 7}    // Move right to exit
};

// Path 1: Bottom path (row 15) -> right -> up -> right to exit
Vector2 path_1_waypoints[] = {
    (Vector2) {0, 15},   // Start at bottom-left of path
    (Vector2) {19, 15},  // Move right to column 19
    (Vector2) {19, 10},  // Move up to row 10
    (Vector2) {24, 10}   // Move right to exit
};

const int path_0_count = sizeof(path_0_waypoints) / sizeof(path_0_waypoints[0]);
const int path_1_count = sizeof(path_1_waypoints) / sizeof(path_1_waypoints[0]);

// Array of path pointers for easy access
Vector2* paths[] = { path_0_waypoints, path_1_waypoints };
const int path_counts[] = { path_0_count, path_1_count };

// Helper function to get stats for each enemy type
typedef struct {
    float health;
    float speed;
} ENEMY_STATS;

ENEMY_STATS get_enemy_stats(const ENEMY_TYPE type) {
    switch (type) {
        case ENEMY_TYPE_SCOUT:
            return (ENEMY_STATS) { .health = 50.0f, .speed = 3.5f };   // Fast, low health
        case ENEMY_TYPE_NORMAL:
            return (ENEMY_STATS) { .health = 100.0f, .speed = 2.5f };  // Balanced
        case ENEMY_TYPE_TANK:
            return (ENEMY_STATS) { .health = 200.0f, .speed = 1.5f };  // Slow, high health
        case ENEMY_TYPE_SPEEDY:
            return (ENEMY_STATS) { .health = 40.0f, .speed = 4.5f };   // Very fast, very low health
        default:
            return (ENEMY_STATS) { .health = 100.0f, .speed = 2.5f };
    }
}

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
    game.enemy_spawn_timer = ENEMY_DEFAULT_TIMER;
    game.next_id = 0;

    game.assets.towers = LoadTexture(ASSETS_PATH "images/towers.png");
    game.assets.enemies = LoadTexture(ASSETS_PATH "assets/characters.png");
    if (game.assets.towers.id == 0) {
        fprintf(stderr, "error: failed to load towers texture\n");
        free(game.game_objects);
        unload_tilemap(&game.tilemap);
        exit(1);
    }

    if (game.assets.enemies.id == 0) {
        fprintf(stderr, "error: failed to load enemies texture\n");
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
      .is_active = true,
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
    if (game == nullptr) {
        exit(1);
    }

    if (game->game_objects == nullptr) {
        exit(1);
    }

    while (!WindowShouldClose())
    {

        const float delta_time = GetFrameTime();
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
        game->enemy_spawn_timer -= delta_time;

        if (game->enemy_spawn_timer <= 0) {
            const int chosen_path = GetRandomValue(0, 1);

            const auto enemy_type = (ENEMY_TYPE)GetRandomValue(0, ENEMY_TYPE_COUNT - 1);

            const ENEMY_STATS stats = get_enemy_stats(enemy_type);

            Vector2 start_pos;
            if (chosen_path == 0) {
                start_pos = path_0_waypoints[0];
            } else {
                start_pos = path_1_waypoints[0];
            }

            add_game_object(game, (GAME_OBJECT) {
                .type = ENEMY,
                .position = start_pos,
                .is_active = true,
                .data.enemy = {
                    .health = stats.health,
                    .max_health = stats.health,
                    .speed = stats.speed,
                    .waypoint_index = 0,
                    .path_id = chosen_path,
                    .type = enemy_type
                }
            });

            game->enemy_spawn_timer = ENEMY_DEFAULT_TIMER;
        }
        BeginDrawing();
        ClearBackground(BLACK);

        draw_tilemap(&game->tilemap);
        draw_game_objects(game);

        update_game_state(game,delta_time);

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

SPRITE_INFO get_tower_sprites(const TOWER_LEVEL level) {
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
    SPRITE_INFO info = { .sprites = nullptr, .count = 0, .width = 0, .height = 0 };

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

SPRITE_INFO get_enemy_sprites(const ENEMY_TYPE type) {
    static constexpr int scout_sprites[] = { 0 };
    static constexpr int normal_sprites[] = { 1 };
    static constexpr int tank_sprites[] = { 2 };
    static constexpr int speedy_sprites[] = { 3 };

    SPRITE_INFO info = { .sprites = nullptr, .count = 0, .width = 0, .height = 0 };

    switch (type) {
        case ENEMY_TYPE_SCOUT:
            info.sprites = scout_sprites;
            info.count = sizeof(scout_sprites) / sizeof(scout_sprites[0]);
            break;
        case ENEMY_TYPE_NORMAL:
            info.sprites = normal_sprites;
            info.count = sizeof(normal_sprites) / sizeof(normal_sprites[0]);
            break;
        case ENEMY_TYPE_TANK:
            info.sprites = tank_sprites;
            info.count = sizeof(tank_sprites) / sizeof(tank_sprites[0]);
            break;
        case ENEMY_TYPE_SPEEDY:
            info.sprites = speedy_sprites;
            info.count = sizeof(speedy_sprites) / sizeof(speedy_sprites[0]);
            break;
        default:
            fprintf(stderr, "WARNING: Unknown enemy type\n");
            info.sprites = normal_sprites;
            info.count = 1;
    }

    info.width = 1;
    info.height = 1;
    return info;
}

void update_game_state(GAME *game, const float delta_time) {
    for (int i = 0; i < game->object_count; i++ ) {
        switch (game->game_objects[i].type) {
            case ENEMY:
                update_enemy(&game->game_objects[i],delta_time);
                break;
            case TOWER:
                break;
            case ANIMATION:
                break;
            case PROJECTILE:
                break;
        }
    }

    remove_inactive_objects(game);
}

void update_enemy(GAME_OBJECT *enemy,float delta_time) {
    if (!enemy->is_active) {
        return;
    }

    const int path_id = enemy->data.enemy.path_id;
    const int waypoint_count = path_counts[path_id];
    const Vector2* current_path = paths[path_id];

    if (enemy->data.enemy.waypoint_index >= waypoint_count) {
        enemy->is_active = false;
        return;
    }

    const Vector2 target_pos = current_path[enemy->data.enemy.waypoint_index];

    const Vector2 direction = {
        target_pos.x - enemy->position.x,
        target_pos.y - enemy->position.y
    };

    const float distance = sqrtf(direction.x * direction.x + direction.y * direction.y);

    if (distance < 0.01f) {
        enemy->position = target_pos;
        enemy->data.enemy.waypoint_index++;
    } else {
        const float abs_dx = fabsf(direction.x);
        const float abs_dy = fabsf(direction.y);

        const float distance_to_move = enemy->data.enemy.speed * delta_time;

        if (abs_dx > abs_dy) {
            if (abs_dx <= distance_to_move) {
                enemy->position.x = target_pos.x;
            } else {
                enemy->position.x += (direction.x > 0 ? 1.0f : -1.0f) * distance_to_move;
            }
        } else {
            if (abs_dy <= distance_to_move) {
                enemy->position.y = target_pos.y;
            } else {
                enemy->position.y += (direction.y > 0 ? 1.0f : -1.0f) * distance_to_move;
            }
        }
    }
}

void remove_inactive_objects(GAME *game) {
    size_t write_index = 0;
    for (size_t read_index = 0; read_index < game->object_count; read_index++) {
        const GAME_OBJECT* obj = &game->game_objects[read_index];

        if (!obj->is_active) {
            if (obj->type == ENEMY) {
                game->player_lives--;
                printf("Enemy reached the end! Lives remaining: %d\n", game->player_lives);
            }
            continue;
        }

        if (write_index != read_index) {
            game->game_objects[write_index] = game->game_objects[read_index];
        }
        write_index++;
    }

    const size_t removed = game->object_count - write_index;
    game->object_count = write_index;

    if (removed > 0) {
        printf("Cleaned up %zu inactive object(s). Active objects: %zu/%zu\n",
               removed, game->object_count, game->object_capacity);
    }

    if (game->object_capacity > STARTING_COUNT_OF_GAME_OBJECTS * 2 &&
        game->object_count < game->object_capacity / 4) {

        const size_t new_capacity = game->object_capacity / 2;

        GAME_OBJECT* temp = realloc(game->game_objects, sizeof(GAME_OBJECT) * new_capacity);

        if (temp != nullptr) {
            game->game_objects = temp;
            game->object_capacity = new_capacity;
            printf("Compacted array from %zu to %zu capacity\n",
                   game->object_capacity * 2, new_capacity);
        } else {
            fprintf(stderr, "WARNING: Failed to compact object array\n");
        }
    }
}

void draw_game_objects(const GAME* game) {
    for (size_t i = 0; i < game->object_count; i++) {
        const GAME_OBJECT* obj = &game->game_objects[i];

        if (obj->type == TOWER) {
            const SPRITE_INFO info = get_tower_sprites(obj->data.tower.level);

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
        if (obj->type == ENEMY) {
            const SPRITE_INFO info = get_enemy_sprites(obj->data.enemy.type);

            if (info.sprites == nullptr) {
                continue;
            }

            for (int y = 0; y < info.height; y++) {
                for (int x = 0; x < info.width; x++) {
                    const int sprite = info.sprites[info.width * y + x];
                    draw_texture(&game->tilemap,game->assets.enemies, sprite, (int)obj->position.x + x, (int)obj->position.y + y);
                }
            }
        }
    }
}
