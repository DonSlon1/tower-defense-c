//
// Created by lukas on 10/12/25.
//
#include "game.h"
#include "tower.h"
#include "enemy.h"
#include "renderer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void handle_playing_input(GAME *game) {
    const GRID_COORD grid_pos = screen_to_grid(GetMousePosition(), &game->tilemap);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const UPGRADE_RESULT result = upgrade_clicked_tower(game, grid_pos);
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
}

void spawn_enemy(GAME *game) {
    const int chosen_path = GetRandomValue(0, 1);
    const auto enemy_type = (ENEMY_TYPE)GetRandomValue(0, ENEMY_TYPE_COUNT - 1);
    const ENEMY_STATS stats = get_enemy_stats(enemy_type);

    // Get start position from enemy.c paths
    extern Vector2 path_0_waypoints[];
    extern Vector2 path_1_waypoints[];
    const Vector2 start_pos = chosen_path == 0 ? path_0_waypoints[0] : path_1_waypoints[0];

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
}

void reset_game(GAME *game) {
    game->player_lives = STARTING_AMOUNT_OF_LIVES;
    game->player_money = STARTING_AMOUNT_OF_MONEY;
    game->enemies_defeated = 0;
    game->enemy_spawn_timer = ENEMY_DEFAULT_TIMER;

    for (size_t i = 0; i < game->object_count; i++) {
        if (game->game_objects[i].type == ENEMY) {
            game->game_objects[i].is_active = false;
        }
    }
    remove_inactive_objects(game);
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
    game.state = GAME_STATE_START;
    game.enemies_defeated = 0;

    game.assets.towers = LoadTexture(ASSETS_PATH "images/towers.png");
    game.assets.enemies = LoadTexture(ASSETS_PATH "assets/characters.png");
    game.assets.start_screen = LoadTexture(ASSETS_PATH "images/start_screen.png");
    game.assets.defeat_screen = LoadTexture(ASSETS_PATH "images/defeat_screen.png");

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

    if (game.assets.start_screen.id == 0) {
        fprintf(stderr, "error: failed to load start screen texture\n");
        free(game.game_objects);
        unload_tilemap(&game.tilemap);
        exit(1);
    }

    if (game.assets.defeat_screen.id == 0) {
        fprintf(stderr, "error: failed to load defeat screen texture\n");
        free(game.game_objects);
        unload_tilemap(&game.tilemap);
        exit(1);
    }

    add_game_object(&game, init_tower((Vector2){5,3}));
    add_game_object(&game, init_tower((Vector2){8,11}));
    add_game_object(&game, init_tower((Vector2){20,3}));
    add_game_object(&game, init_tower((Vector2){20,11}));

    return game;
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
    if (game == nullptr || game->game_objects == nullptr) exit(1);

    while (!WindowShouldClose()) {
        const float delta_time = GetFrameTime();

        if (game->state == GAME_STATE_START && IsKeyPressed(KEY_SPACE)) {
            game->state = GAME_STATE_PLAYING;
        } else if (game->state == GAME_STATE_PLAYING) {
            if (game->player_lives <= 0) game->state = GAME_STATE_GAME_OVER;

            handle_playing_input(game);
            game->enemy_spawn_timer -= delta_time;
            if (game->enemy_spawn_timer <= 0) {
                spawn_enemy(game);
                game->enemy_spawn_timer = ENEMY_DEFAULT_TIMER;
            }
            update_game_state(game, delta_time);
        } else if (game->state == GAME_STATE_GAME_OVER && IsKeyPressed(KEY_SPACE)) {
            reset_game(game);
            game->state = GAME_STATE_PLAYING;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        draw_tilemap(&game->tilemap);
        draw_game_objects(game);

        if (game->state == GAME_STATE_START) draw_start_screen(game);
        else if (game->state == GAME_STATE_PLAYING) draw_hud(game);
        else if (game->state == GAME_STATE_GAME_OVER) {
            draw_hud(game);
            draw_game_over_screen(game);
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
    UnloadTexture(game->assets.enemies);
    UnloadTexture(game->assets.start_screen);
    UnloadTexture(game->assets.defeat_screen);
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

void update_game_state(GAME *game, const float delta_time) {
    for (int i = 0; i < game->object_count; i++ ) {
        switch (game->game_objects[i].type) {
            case ENEMY:
                update_enemy(&game->game_objects[i], delta_time);
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

void remove_inactive_objects(GAME *game) {
    size_t write_index = 0;
    for (size_t read_index = 0; read_index < game->object_count; read_index++) {
        const GAME_OBJECT* obj = &game->game_objects[read_index];

        if (!obj->is_active) {
            if (obj->type == ENEMY) {
                if (obj->data.enemy.health <= 0) {
                    game->enemies_defeated++;
                    printf("Enemy defeated! Score: %d\n", game->enemies_defeated);
                }
                else {
                    game->player_lives--;
                    printf("Enemy reached the end! Lives remaining: %d\n", game->player_lives);
                }
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
