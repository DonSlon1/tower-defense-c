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

void handle_playing_input(game *game) {
    if (game == nullptr) return;

    const grid_coord grid_pos = screen_to_grid(get_mouse_position(), &game->tilemap);

    const game_object* hovered_tower = find_tower_at_grid(game, grid_pos);
    const int spot_index = find_tower_spot_at_grid(game, grid_pos);

    if (hovered_tower != nullptr || spot_index >= 0) {
        use_pointer_cursor();
    } else {
        use_normal_cursor();
    }

    if (is_mouse_button_pressed(mouse_button_left)) {
        const upgrade_result result = upgrade_clicked_tower(game, grid_pos);
        switch (result) {
            case upgrade_success:
                printf("Tower upgraded successfully!\n");
                break;
            case upgrade_insufficient_funds:
                printf("Insufficient funds for upgrade\n");
                break;
            case upgrade_max_level:
                printf("Tower is already at max level\n");
                break;
            case upgrade_not_found:
                if (spot_index >= 0) {
                    try_build_tower(game, spot_index);
                }
                break;
        }
    }
}

wave_config get_wave_config(const int wave_number) {
    const wave_config waves[] = {
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
        return (wave_config){
            .enemy_count = 35 + (wave_number - MAX_WAVES + 1) * 5,
            .spawn_interval = 0.5f,
            .flying_chance = 70,
            .allow_bottom_path = true
        };
    }

    return waves[wave_number];
}

void spawn_enemy(game *game) {
    if (game == nullptr) return;

    const wave_config wave = get_wave_config(game->current_wave);

    int chosen_path;
    if (wave.allow_bottom_path) {
        chosen_path = get_random_value(0, 1);
    } else {
        chosen_path = 0;
    }

    const enemy_type enemy_type = get_random_value(1, 100) <= wave.flying_chance ? enemy_type_flying : enemy_type_mushroom;
    const enemy_stats stats = get_enemy_stats(enemy_type);

    const vector2 start_pos = get_path_start_position(chosen_path);

    add_game_object(game, (game_object) {
        .type = enemy,
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
            .anim_state = enemy_anim_run,
            .current_frame = 0,
            .frame_timer = 0.0f
        }
    });

    game->enemies_spawned_in_wave++;
    game->enemies_alive++;
}

void start_next_wave(game *game) {
    if (game == nullptr) return;

    game->current_wave++;
    game->enemies_spawned_in_wave = 0;
    game->enemies_alive = 0;

    const wave_config wave = get_wave_config(game->current_wave);
    game->enemy_spawn_timer = wave.spawn_interval;
}

void reset_game(game *game) {
    if (game == nullptr) return;

    game->player_lives = STARTING_AMOUNT_OF_LIVES;
    game->player_money = STARTING_AMOUNT_OF_MONEY;
    game->enemies_defeated = 0;
    game->current_wave = -1;
    game->enemies_spawned_in_wave = 0;
    game->enemies_alive = 0;
    game->wave_break_timer = 0.0f;

    for (size_t i = 0; i < game->object_count; i++) {
        if (game->game_objects[i].type == enemy || game->game_objects[i].type == projectile) {
            game->game_objects[i].is_active = false;
        }
    }
    remove_inactive_objects(game);
}

game init_game() {
    game game;
    game.tilemap = init_tilemap();
    game.game_objects =  malloc(sizeof(game_object) * STARTING_COUNT_OF_GAME_OBJECTS);
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
    game.state = game_state_start;
    game.enemies_defeated = 0;

    game.current_wave = -1;
    game.enemies_spawned_in_wave = 0;
    game.enemies_alive = 0;
    game.wave_break_timer = 0.0f;

    game.tower_spots[0] = (tower_spot){.position = (vector2){5, 3}, .occupied = false};
    game.tower_spots[1] = (tower_spot){.position = (vector2){8, 11}, .occupied = false};
    game.tower_spots[2] = (tower_spot){.position = (vector2){15, 3}, .occupied = false};
    game.tower_spots[3] = (tower_spot){.position = (vector2){15, 11}, .occupied = false};

    game.assets.towers = load_texture(ASSETS_PATH "images/towers.png");
    game.assets.mushroom_run = load_texture(ASSETS_PATH "images/Mushroom-Run.png");
    game.assets.mushroom_hit = load_texture(ASSETS_PATH "images/Mushroom-Hit.png");
    game.assets.mushroom_die = load_texture(ASSETS_PATH "images/Mushroom-Die.png");
    game.assets.flying_fly = load_texture(ASSETS_PATH "images/Enemy3-Fly.png");
    game.assets.flying_hit = load_texture(ASSETS_PATH "images/Enemy3-Hit.png");
    game.assets.flying_die = load_texture(ASSETS_PATH "images/Enemy3-Die.png");
    game.assets.iceball = load_texture(ASSETS_PATH "images/Iceball_84x9.png");
    game.assets.start_screen = load_texture(ASSETS_PATH "images/start_screen.png");
    game.assets.defeat_screen = load_texture(ASSETS_PATH "images/defeat_screen.png");

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

void add_game_object(game *game, game_object game_object) {
    if (game == nullptr || game->game_objects == nullptr) return;

    if (game->object_count == game->object_capacity) {
        grow_object_capacity(game);
    }

    game_object.id = game->next_id;
    game->game_objects[game->object_count] = game_object;
    game->object_count++;
    game->next_id++;
}

void grow_object_capacity(game* game) {
    if (game == nullptr) return;

    size_t new_capacity = game->object_capacity * 2;
    if (new_capacity == 0) new_capacity = STARTING_COUNT_OF_GAME_OBJECTS;

    if (new_capacity < game->object_capacity) {
        fprintf(stderr, "ERROR: Capacity overflow when resizing game objects array.\n");
        exit(1);
    }

    game_object* temp = realloc(game->game_objects, sizeof(game_object) * new_capacity);

    if (temp == nullptr) {
        fprintf(stderr, "ERROR: Failed to reallocate memory for game objects.\n");
        exit(1);
    }

    game->game_objects = temp;
    game->object_capacity = new_capacity;
}

void start_game(game *game) {
    if (game == nullptr || game->game_objects == nullptr) exit(1);

    while (!window_should_close()) {
        const float delta_time = get_frame_time();

        if (game->state == game_state_start && is_key_pressed(key_space)) {
            start_next_wave(game);
            game->state = game_state_playing;
        }
        else if (game->state == game_state_playing) {
            if (game->player_lives <= 0) game->state = game_state_game_over;

            handle_playing_input(game);

            const wave_config current_wave = get_wave_config(game->current_wave);

            if (game->enemies_spawned_in_wave < current_wave.enemy_count) {
                game->enemy_spawn_timer -= delta_time;
                if (game->enemy_spawn_timer <= 0) {
                    spawn_enemy(game);
                    game->enemy_spawn_timer = current_wave.spawn_interval;
                }
            }

            else if (game->enemies_alive == 0) {
                game->state = game_state_wave_break;
                game->wave_break_timer = WAVE_BREAK_DURATION;
            }

            update_game_state(game, delta_time);
        }
        else if (game->state == game_state_wave_break) {
            handle_playing_input(game);
            update_game_state(game, delta_time);

            game->wave_break_timer -= delta_time;
            if (game->wave_break_timer <= 0 || is_key_pressed(key_space)) {
                start_next_wave(game);
                game->state = game_state_playing;
            }
        }
        else if (game->state == game_state_game_over && is_key_pressed(key_space)) {
            reset_game(game);
            start_next_wave(game);
            game->state = game_state_playing;
        }

        begin_drawing();
        clear_background(black);
        draw_tilemap(&game->tilemap);

        if (game->state == game_state_playing) {
            draw_tower_spots(game);
        }

        draw_game_objects(game);

        if (game->state == game_state_start) {
            draw_start_screen(game);
        }
        else if (game->state == game_state_playing) {
            draw_hud(game);
            draw_wave_info(game);

            const grid_coord mouse_grid = screen_to_grid(get_mouse_position(), &game->tilemap);
            const game_object* hovered_tower = find_tower_at_grid(game, mouse_grid);
            if (hovered_tower != nullptr) {
                const vector2 mouse_pos = get_mouse_position();
                draw_tower_info(hovered_tower, (int)mouse_pos.x + 15, (int)mouse_pos.y + 15);
            }
        }
        else if (game->state == game_state_wave_break) {
            draw_hud(game);
            draw_wave_info(game);
            draw_wave_break_screen(game);
        }
        else if (game->state == game_state_game_over) {
            draw_hud(game);
            draw_game_over_screen(game);
        }

        draw_fps(10, 10);
        end_drawing();
    }
    unload_game(game);
}

void unload_game(game *game) {
    if (game == nullptr) return;

    unload_tilemap(&game->tilemap);
    free(game->game_objects);
    unload_texture(game->assets.towers);
    unload_texture(game->assets.mushroom_run);
    unload_texture(game->assets.mushroom_hit);
    unload_texture(game->assets.mushroom_die);
    unload_texture(game->assets.flying_fly);
    unload_texture(game->assets.flying_hit);
    unload_texture(game->assets.flying_die);
    unload_texture(game->assets.iceball);
    unload_texture(game->assets.start_screen);
    unload_texture(game->assets.defeat_screen);
    game->game_objects = nullptr;
    printf("GAME: was unloaded\n");
}

grid_coord screen_to_grid(const vector2 screen_pos, const tile_map* tilemap) {
    if (tilemap == nullptr) {
        return (grid_coord){.x = 0, .y = 0};
    }

    const int scaled_tile_size = get_tile_scale(tilemap);

    grid_coord grid_pos;
    grid_pos.x = (int)(screen_pos.x / (float)scaled_tile_size);
    grid_pos.y = (int)(screen_pos.y / (float)scaled_tile_size);

    return grid_pos;
}

/**
 * IMPORTANT: The caller is responsible for freeing the memory allocated for 'out_objects'
 * @return The number of objects found, or -1 on memory allocation failure.
 */
int get_game_objects_of_type(const game *game, const object_type type, game_object **out_objects) {
    if (game == nullptr || out_objects == nullptr) {
        return -1;
    }

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

    *out_objects = malloc(sizeof(game_object) * count);

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

void update_game_state(game *game, const float delta_time) {
    if (game == nullptr || game->game_objects == nullptr) return;

    for (int i = 0; i < game->object_count; i++ ) {
        switch (game->game_objects[i].type) {
            case enemy:
                update_enemy(&game->game_objects[i], delta_time);
                break;
            case tower:
                update_tower(game, &game->game_objects[i], delta_time);
                break;
            case projectile:
                update_projectile(game, &game->game_objects[i], delta_time);
                break;
        }
    }

    remove_inactive_objects(game);
}

int find_tower_spot_at_grid(const game *game, const grid_coord grid_coord) {
    if (game == nullptr) return -1;

    for (int i = 0; i < 4; i++) {
        const tower_spot* spot = &game->tower_spots[i];
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

bool try_build_tower(game *game, const int spot_index) {
    if (game == nullptr) return false;

    if (spot_index < 0 || spot_index >= 4) {
        fprintf(stderr, "ERROR: Invalid tower spot index: %d\n", spot_index);
        return false;
    }

    tower_spot* spot = &game->tower_spots[spot_index];

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

game_object* find_tower_at_grid(const game *game, const grid_coord grid_coord) {
    if (game == nullptr || game->game_objects == nullptr) return nullptr;

    for (size_t i = 0; i < game->object_count; i++) {
        if (game->game_objects[i].type != tower) {
            continue;
        }

        game_object* tower = &game->game_objects[i];

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

void remove_inactive_objects(game *game) {
    if (game == nullptr || game->game_objects == nullptr) return;

    size_t write_index = 0;
    for (size_t read_index = 0; read_index < game->object_count; read_index++) {
        const game_object* obj = &game->game_objects[read_index];

        if (!obj->is_active) {
            if (obj->type == enemy) {
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

    if (game->object_capacity > (size_t)STARTING_COUNT_OF_GAME_OBJECTS * 2 &&
        game->object_count < game->object_capacity / 4) {

        const size_t new_capacity = game->object_capacity / 2;

        game_object* temp = realloc(game->game_objects, sizeof(game_object) * new_capacity);

        if (temp != nullptr) {
            game->game_objects = temp;
            game->object_capacity = new_capacity;
        } else {
            fprintf(stderr, "WARNING: Failed to compact object array\n");
        }
    }
}
