//
// Created by lukas on 10/12/25.
//
#include "game.h"
#include "tower.h"
#include "enemy.h"
#include "renderer.h"
#include "projectile.h"

#include <stdio.h>
#include <stdlib.h>

void handle_playing_input(GAME *game) {
    const GRID_COORD grid_pos = screen_to_grid(GetMousePosition(), &game->tilemap);

    const GAME_OBJECT* hovered_tower = find_tower_at_grid(game, grid_pos);
    const int spot_index = find_tower_spot_at_grid(game, grid_pos);

    if (hovered_tower != nullptr || spot_index >= 0) {
        UsePointerCursor();
    } else {
        UseNormalCursor();
    }

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
                if (spot_index >= 0) {
                    try_build_tower(game, spot_index);
                }
                break;
        }
    }
}

WAVE_CONFIG get_wave_config(const int wave_number) {
    const WAVE_CONFIG waves[] = {
        {.enemy_count = 5, .spawn_interval = 2.0f, .flying_chance = 20, .allow_bottom_path = false},
        {.enemy_count = 8, .spawn_interval = 1.8f, .flying_chance = 25, .allow_bottom_path = false},
        {.enemy_count = 10, .spawn_interval = 1.6f, .flying_chance = 30, .allow_bottom_path = true},
        {.enemy_count = 12, .spawn_interval = 1.4f, .flying_chance = 35, .allow_bottom_path = true},
        {.enemy_count = 15, .spawn_interval = 1.2f, .flying_chance = 40, .allow_bottom_path = true},
        {.enemy_count = 18, .spawn_interval = 1.0f, .flying_chance = 45, .allow_bottom_path = true},
        {.enemy_count = 22, .spawn_interval = 0.9f, .flying_chance = 50, .allow_bottom_path = true},
        {.enemy_count = 25, .spawn_interval = 0.8f, .flying_chance = 55, .allow_bottom_path = true},
        {.enemy_count = 30, .spawn_interval = 0.7f, .flying_chance = 60, .allow_bottom_path = true},
        {.enemy_count = 35, .spawn_interval = 0.6f, .flying_chance = 65, .allow_bottom_path = true}
    };

    if (wave_number >= MAX_WAVES) {
        return (WAVE_CONFIG){
            .enemy_count = 35 + (wave_number - MAX_WAVES + 1) * 5,
            .spawn_interval = 0.5f,
            .flying_chance = 70,
            .allow_bottom_path = true
        };
    }

    return waves[wave_number];
}

void spawn_enemy(GAME *game) {
    const WAVE_CONFIG wave = get_wave_config(game->current_wave);

    int chosen_path;
    if (wave.allow_bottom_path) {
        chosen_path = GetRandomValue(0, 1);
    } else {
        chosen_path = 0;
    }

    const ENEMY_TYPE enemy_type = GetRandomValue(1, 100) <= wave.flying_chance ? ENEMY_TYPE_FLYING : ENEMY_TYPE_MUSHROOM;
    const ENEMY_STATS stats = get_enemy_stats(enemy_type);

    const Vector2 start_pos = get_path_start_position(chosen_path);

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
            .gold_reward = stats.gold_reward,
            .type = enemy_type,
            .anim_state = ENEMY_ANIM_RUN,
            .current_frame = 0,
            .frame_timer = 0.0f
        }
    });

    game->enemies_spawned_in_wave++;
    game->enemies_alive++;
}

void start_next_wave(GAME *game) {
    game->current_wave++;
    game->enemies_spawned_in_wave = 0;
    game->enemies_alive = 0;

    const WAVE_CONFIG wave = get_wave_config(game->current_wave);
    game->enemy_spawn_timer = wave.spawn_interval;
}

void reset_game(GAME *game) {
    game->player_lives = STARTING_AMOUNT_OF_LIVES;
    game->player_money = STARTING_AMOUNT_OF_MONEY;
    game->enemies_defeated = 0;
    game->current_wave = -1;
    game->enemies_spawned_in_wave = 0;
    game->enemies_alive = 0;
    game->wave_break_timer = 0.0f;

    for (size_t i = 0; i < game->object_count; i++) {
        if (game->game_objects[i].type == ENEMY || game->game_objects[i].type == PROJECTILE) {
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
    game.enemy_spawn_timer = 0.0f;
    game.next_id = 0;
    game.state = GAME_STATE_START;
    game.enemies_defeated = 0;

    game.current_wave = -1;
    game.enemies_spawned_in_wave = 0;
    game.enemies_alive = 0;
    game.wave_break_timer = 0.0f;

    game.tower_spots[0] = (TOWER_SPOT){.position = (Vector2){5, 3}, .occupied = false};
    game.tower_spots[1] = (TOWER_SPOT){.position = (Vector2){8, 11}, .occupied = false};
    game.tower_spots[2] = (TOWER_SPOT){.position = (Vector2){15, 3}, .occupied = false};
    game.tower_spots[3] = (TOWER_SPOT){.position = (Vector2){15, 11}, .occupied = false};

    game.assets.towers = LoadTexture(ASSETS_PATH "images/towers.png");
    game.assets.mushroom_run = LoadTexture(ASSETS_PATH "images/Mushroom-Run.png");
    game.assets.mushroom_hit = LoadTexture(ASSETS_PATH "images/Mushroom-Hit.png");
    game.assets.mushroom_die = LoadTexture(ASSETS_PATH "images/Mushroom-Die.png");
    game.assets.flying_fly = LoadTexture(ASSETS_PATH "images/Enemy3-Fly.png");
    game.assets.flying_hit = LoadTexture(ASSETS_PATH "images/Enemy3-Hit.png");
    game.assets.flying_die = LoadTexture(ASSETS_PATH "images/Enemy3-Die.png");
    game.assets.iceball = LoadTexture(ASSETS_PATH "images/Iceball_84x9.png");
    game.assets.start_screen = LoadTexture(ASSETS_PATH "images/start_screen.png");
    game.assets.defeat_screen = LoadTexture(ASSETS_PATH "images/defeat_screen.png");

    if (game.assets.towers.id == 0) {
        fprintf(stderr, "error: failed to load towers texture\n");
        free(game.game_objects);
        unload_tilemap(&game.tilemap);
        exit(1);
    }

    if (game.assets.mushroom_run.id == 0 || game.assets.flying_fly.id == 0) {
        fprintf(stderr, "error: failed to load enemy textures\n");
        free(game.game_objects);
        unload_tilemap(&game.tilemap);
        exit(1);
    }

    if (game.assets.iceball.id == 0) {
        fprintf(stderr, "error: failed to load projectile texture\n");
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

    add_game_object(&game, init_tower(game.tower_spots[0].position));
    game.tower_spots[0].occupied = true;

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
            start_next_wave(game);
            game->state = GAME_STATE_PLAYING;
        }
        else if (game->state == GAME_STATE_PLAYING) {
            if (game->player_lives <= 0) game->state = GAME_STATE_GAME_OVER;

            handle_playing_input(game);

            const WAVE_CONFIG current_wave = get_wave_config(game->current_wave);

            if (game->enemies_spawned_in_wave < current_wave.enemy_count) {
                game->enemy_spawn_timer -= delta_time;
                if (game->enemy_spawn_timer <= 0) {
                    spawn_enemy(game);
                    game->enemy_spawn_timer = current_wave.spawn_interval;
                }
            }

            else if (game->enemies_alive == 0) {
                game->state = GAME_STATE_WAVE_BREAK;
                game->wave_break_timer = WAVE_BREAK_DURATION;
            }

            update_game_state(game, delta_time);
        }
        else if (game->state == GAME_STATE_WAVE_BREAK) {
            handle_playing_input(game);
            update_game_state(game, delta_time);

            game->wave_break_timer -= delta_time;
            if (game->wave_break_timer <= 0 || IsKeyPressed(KEY_SPACE)) {
                start_next_wave(game);
                game->state = GAME_STATE_PLAYING;
            }
        }
        else if (game->state == GAME_STATE_GAME_OVER && IsKeyPressed(KEY_SPACE)) {
            reset_game(game);
            start_next_wave(game);
            game->state = GAME_STATE_PLAYING;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        draw_tilemap(&game->tilemap);

        if (game->state == GAME_STATE_PLAYING) {
            draw_tower_spots(game);
        }

        draw_game_objects(game);

        if (game->state == GAME_STATE_START) {
            draw_start_screen(game);
        }
        else if (game->state == GAME_STATE_PLAYING) {
            draw_hud(game);
            draw_wave_info(game);

            const GRID_COORD mouse_grid = screen_to_grid(GetMousePosition(), &game->tilemap);
            const GAME_OBJECT* hovered_tower = find_tower_at_grid(game, mouse_grid);
            if (hovered_tower != nullptr) {
                const Vector2 mouse_pos = GetMousePosition();
                draw_tower_info(hovered_tower, (int)mouse_pos.x + 15, (int)mouse_pos.y + 15);
            }
        }
        else if (game->state == GAME_STATE_WAVE_BREAK) {
            draw_hud(game);
            draw_wave_info(game);
            draw_wave_break_screen(game);
        }
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
    UnloadTexture(game->assets.mushroom_run);
    UnloadTexture(game->assets.mushroom_hit);
    UnloadTexture(game->assets.mushroom_die);
    UnloadTexture(game->assets.flying_fly);
    UnloadTexture(game->assets.flying_hit);
    UnloadTexture(game->assets.flying_die);
    UnloadTexture(game->assets.iceball);
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
                update_tower(game, &game->game_objects[i], delta_time);
                break;
            case PROJECTILE:
                update_projectile(game, &game->game_objects[i], delta_time);
                break;
        }
    }

    remove_inactive_objects(game);
}

int find_tower_spot_at_grid(const GAME *game, const GRID_COORD grid_coord) {
    for (int i = 0; i < 4; i++) {
        const TOWER_SPOT* spot = &game->tower_spots[i];
        const int spot_x = (int)spot->position.x;
        const int spot_y = (int)spot->position.y;

        const bool is_inside = grid_coord.x >= spot_x &&
                               grid_coord.x < spot_x + 4 &&
                               grid_coord.y >= spot_y &&
                               grid_coord.y < spot_y + 4;

        if (is_inside) {
            return i;
        }
    }
    return -1;
}

bool try_build_tower(GAME *game, const int spot_index) {
    if (spot_index < 0 || spot_index >= 4) {
        fprintf(stderr, "ERROR: Invalid tower spot index: %d\n", spot_index);
        return false;
    }

    TOWER_SPOT* spot = &game->tower_spots[spot_index];

    if (spot->occupied) {
        return false;
    }

    if (game->player_money < TOWER_BUILD_COST) {
        printf("Not enough money to build tower! Need $%d, have $%d\n",
               TOWER_BUILD_COST, game->player_money);
        return false;
    }

    game->player_money -= TOWER_BUILD_COST;
    add_game_object(game, init_tower(spot->position));
    spot->occupied = true;
    printf("Tower built! Cost: $%d, Money remaining: $%d\n",
           TOWER_BUILD_COST, game->player_money);

    return true;
}

GAME_OBJECT* find_tower_at_grid(const GAME *game, const GRID_COORD grid_coord) {
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
            return tower;
        }
    }
    return nullptr;
}

void remove_inactive_objects(GAME *game) {
    size_t write_index = 0;
    for (size_t read_index = 0; read_index < game->object_count; read_index++) {
        const GAME_OBJECT* obj = &game->game_objects[read_index];

        if (!obj->is_active) {
            if (obj->type == ENEMY) {
                game->enemies_alive--;
                if (obj->data.enemy.health <= 0) {
                    game->enemies_defeated++;
                    game->player_money += obj->data.enemy.gold_reward;
                }
                else {
                    game->player_lives--;
                }
            }
            continue;
        }

        if (write_index != read_index) {
            game->game_objects[write_index] = game->game_objects[read_index];
        }
        write_index++;
    }

    game->object_count = write_index;

    if (game->object_capacity > STARTING_COUNT_OF_GAME_OBJECTS * 2 &&
        game->object_count < game->object_capacity / 4) {

        const size_t new_capacity = game->object_capacity / 2;

        GAME_OBJECT* temp = realloc(game->game_objects, sizeof(GAME_OBJECT) * new_capacity);

        if (temp != nullptr) {
            game->game_objects = temp;
            game->object_capacity = new_capacity;
        } else {
            fprintf(stderr, "WARNING: Failed to compact object array\n");
        }
    }
}
