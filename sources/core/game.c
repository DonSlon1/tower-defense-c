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

void handle_playing_input(game *g) {
    if (g == nullptr) return;

    const grid_coord grid_pos = screen_to_grid(get_mouse_position(), &g->tilemap);

    const game_object* hovered_tower = find_tower_at_grid(g, grid_pos);
    const int spot_index = find_tower_spot_at_grid(g, grid_pos);

    if (hovered_tower != nullptr || spot_index >= 0) {
        use_pointer_cursor();
    } else {
        use_normal_cursor();
    }

    if (is_mouse_button_pressed(mouse_button_left)) {
        const upgrade_result result = upgrade_clicked_tower(g, grid_pos);
        switch (result) {
            case upgrade_success:
            case upgrade_insufficient_funds:
            case upgrade_max_level:
                break;
            case upgrade_not_found:
                if (spot_index >= 0) {
                    try_build_tower(g, spot_index);
                }
                break;
            default:
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

void spawn_enemy(game *g) {
    if (g == nullptr) return;

    const wave_config wave = get_wave_config(g->current_wave);

    int chosen_path;
    if (wave.allow_bottom_path) {
        chosen_path = get_random_value(0, 1);
    } else {
        chosen_path = 0;
    }

    const enemy_type etype = get_random_value(1, 100) <= wave.flying_chance ? enemy_type_flying : enemy_type_mushroom;
    const enemy_stats stats = get_enemy_stats(etype);

    const vector2 start_pos = get_path_start_position(chosen_path);

    const result_code res = add_game_object(g, (game_object) {
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
            .type = etype,
            .anim_state = enemy_anim_run,
            .current_frame = 0,
            .frame_timer = 0.0f
        }
    });

    if (res != result_ok) {
        fprintf(stderr, "ERROR: Failed to spawn enemy: code %u\n", (unsigned)res);
        return;
    }

    g->enemies_spawned_in_wave++;
    g->enemies_alive++;
}

void start_next_wave(game *g) {
    if (g == nullptr) return;

    g->current_wave++;
    g->enemies_spawned_in_wave = 0;
    g->enemies_alive = 0;

    const wave_config wave = get_wave_config(g->current_wave);
    g->enemy_spawn_timer = wave.spawn_interval;
}

void reset_game(game *g) {
    if (g == nullptr) return;

    g->player_lives = STARTING_AMOUNT_OF_LIVES;
    g->player_money = STARTING_AMOUNT_OF_MONEY;
    g->enemies_defeated = 0;
    g->current_wave = -1;
    g->enemies_spawned_in_wave = 0;
    g->enemies_alive = 0;
    g->wave_break_timer = 0.0f;

    for (size_t i = 0; i < g->object_count; i++) {
        if (g->game_objects[i].type == enemy || g->game_objects[i].type == projectile) {
            g->game_objects[i].is_active = false;
        }
    }
    remove_inactive_objects(g);
}

game init_game() {
    game g;
    g.tilemap = init_tilemap();
    g.game_objects =  malloc(sizeof(game_object) * STARTING_COUNT_OF_GAME_OBJECTS);
    if (g.game_objects == nullptr) {
        fprintf(stderr, "ERROR: Failed to allocate memory for game objects\n");
        unload_tilemap(&g.tilemap);
        exit(1);
    }
    g.object_capacity = STARTING_COUNT_OF_GAME_OBJECTS;
    g.object_count = 0;
    g.player_money = STARTING_AMOUNT_OF_MONEY;
    g.player_lives = STARTING_AMOUNT_OF_LIVES;
    g.enemy_spawn_timer = 0.0f;
    g.next_id = 0;
    g.state = game_state_start;
    g.enemies_defeated = 0;

    g.current_wave = -1;
    g.enemies_spawned_in_wave = 0;
    g.enemies_alive = 0;
    g.wave_break_timer = 0.0f;

    g.tower_spots[0] = (tower_spot){.position = (vector2){5, 3}, .occupied = false};
    g.tower_spots[1] = (tower_spot){.position = (vector2){8, 11}, .occupied = false};
    g.tower_spots[2] = (tower_spot){.position = (vector2){15, 3}, .occupied = false};
    g.tower_spots[3] = (tower_spot){.position = (vector2){15, 11}, .occupied = false};

    g.assets.towers = load_texture(ASSETS_PATH "images/towers.png");
    g.assets.mushroom_run = load_texture(ASSETS_PATH "images/Mushroom-Run.png");
    g.assets.mushroom_hit = load_texture(ASSETS_PATH "images/Mushroom-Hit.png");
    g.assets.mushroom_die = load_texture(ASSETS_PATH "images/Mushroom-Die.png");
    g.assets.flying_fly = load_texture(ASSETS_PATH "images/Enemy3-Fly.png");
    g.assets.flying_hit = load_texture(ASSETS_PATH "images/Enemy3-Hit.png");
    g.assets.flying_die = load_texture(ASSETS_PATH "images/Enemy3-Die.png");
    g.assets.iceball = load_texture(ASSETS_PATH "images/Iceball_84x9.png");
    g.assets.start_screen = load_texture(ASSETS_PATH "images/start_screen.png");
    g.assets.defeat_screen = load_texture(ASSETS_PATH "images/defeat_screen.png");

    if (g.assets.towers.id == 0) {
        fprintf(stderr, "error: failed to load towers texture\n");
        free(g.game_objects);
        unload_tilemap(&g.tilemap);
        exit(1);
    }

    if (g.assets.mushroom_run.id == 0 || g.assets.flying_fly.id == 0) {
        fprintf(stderr, "error: failed to load enemy textures\n");
        free(g.game_objects);
        unload_tilemap(&g.tilemap);
        exit(1);
    }

    if (g.assets.iceball.id == 0) {
        fprintf(stderr, "error: failed to load projectile texture\n");
        free(g.game_objects);
        unload_tilemap(&g.tilemap);
        exit(1);
    }

    if (g.assets.start_screen.id == 0) {
        fprintf(stderr, "error: failed to load start screen texture\n");
        free(g.game_objects);
        unload_tilemap(&g.tilemap);
        exit(1);
    }

    if (g.assets.defeat_screen.id == 0) {
        fprintf(stderr, "error: failed to load defeat screen texture\n");
        free(g.game_objects);
        unload_tilemap(&g.tilemap);
        exit(1);
    }

    const result_code res = add_game_object(&g, init_tower(g.tower_spots[0].position));
    if (res != result_ok) {
        fprintf(stderr, "ERROR: Failed to add initial tower: code %u\n", (unsigned)res);
        free(g.game_objects);
        unload_tilemap(&g.tilemap);
        exit(1);
    }
    g.tower_spots[0].occupied = true;

    return g;
}

result_code add_game_object(game *g, game_object obj) {
    VALIDATE_PTR_RET(g, result_error_null_ptr);
    VALIDATE_PTR_RET(g->game_objects, result_error_null_ptr);

    if (g->object_count >= MAX_GAME_OBJECTS) {
        fprintf(stderr, "ERROR: Cannot add object - MAX_GAME_OBJECTS (%zu) limit reached\n", (size_t)MAX_GAME_OBJECTS);
        return result_error_out_of_bounds;
    }

    if (g->object_count == g->object_capacity) {
        const result_code res = grow_object_capacity(g);
        if (res != result_ok) {
            return res;
        }
    }

    obj.id = g->next_id;
    g->game_objects[g->object_count] = obj;
    g->object_count++;
    g->next_id++;

    return result_ok;
}

result_code grow_object_capacity(game* g) {
    VALIDATE_PTR_RET(g, result_error_null_ptr);

    size_t new_capacity = g->object_capacity * 2;
    if (new_capacity == 0) new_capacity = STARTING_COUNT_OF_GAME_OBJECTS;

    if (new_capacity > MAX_GAME_OBJECTS) {
        new_capacity = MAX_GAME_OBJECTS;
    }

    if (new_capacity <= g->object_capacity) {
        fprintf(stderr, "ERROR: Cannot grow capacity beyond MAX_GAME_OBJECTS (%zu)\n", (size_t)MAX_GAME_OBJECTS);
        return result_error_out_of_bounds;
    }

    game_object* temp = realloc(g->game_objects, sizeof(game_object) * new_capacity);

    if (temp == nullptr) {
        fprintf(stderr, "ERROR: Failed to reallocate memory for game objects.\n");
        return result_error_out_of_memory;
    }

    g->game_objects = temp;
    g->object_capacity = new_capacity;

    return result_ok;
}

void start_game(game *g) {
    if (g == nullptr || g->game_objects == nullptr) exit(1);

    while (!window_should_close()) {
        const float delta_time = get_frame_time();

        if (g->state == game_state_start && is_key_pressed(key_space)) {
            start_next_wave(g);
            g->state = game_state_playing;
        }
        else if (g->state == game_state_playing) {
            if (g->player_lives <= 0) g->state = game_state_game_over;

            handle_playing_input(g);

            const wave_config current_wave = get_wave_config(g->current_wave);

            if (g->enemies_spawned_in_wave < current_wave.enemy_count) {
                g->enemy_spawn_timer -= delta_time;
                if (g->enemy_spawn_timer <= 0) {
                    spawn_enemy(g);
                    g->enemy_spawn_timer = current_wave.spawn_interval;
                }
            }

            else if (g->enemies_alive == 0) {
                g->state = game_state_wave_break;
                g->wave_break_timer = WAVE_BREAK_DURATION;
            }

            update_game_state(g, delta_time);
        }
        else if (g->state == game_state_wave_break) {
            handle_playing_input(g);
            update_game_state(g, delta_time);

            g->wave_break_timer -= delta_time;
            if (g->wave_break_timer <= 0 || is_key_pressed(key_space)) {
                start_next_wave(g);
                g->state = game_state_playing;
            }
        }
        else if (g->state == game_state_game_over && is_key_pressed(key_space)) {
            reset_game(g);
            start_next_wave(g);
            g->state = game_state_playing;
        }

        begin_drawing();
        clear_background(black);
        draw_tilemap(&g->tilemap);

        if (g->state == game_state_playing) {
            draw_tower_spots(g);
        }

        draw_game_objects(g);

        if (g->state == game_state_start) {
            draw_start_screen(g);
        }
        else if (g->state == game_state_playing) {
            draw_hud(g);
            draw_wave_info(g);

            const grid_coord mouse_grid = screen_to_grid(get_mouse_position(), &g->tilemap);
            const game_object* hovered_tower = find_tower_at_grid(g, mouse_grid);
            if (hovered_tower != nullptr) {
                const vector2 mouse_pos = get_mouse_position();
                draw_tower_info(hovered_tower, (int)mouse_pos.x + 15, (int)mouse_pos.y + 15);
            }
        }
        else if (g->state == game_state_wave_break) {
            draw_hud(g);
            draw_wave_info(g);
            draw_wave_break_screen(g);
        }
        else if (g->state == game_state_game_over) {
            draw_hud(g);
            draw_game_over_screen(g);
        }

        draw_fps(10, 10);
        end_drawing();
    }
    unload_game(g);
}

void unload_game(game *g) {
    if (g == nullptr) return;

    unload_tilemap(&g->tilemap);
    free(g->game_objects);
    unload_texture(g->assets.towers);
    unload_texture(g->assets.mushroom_run);
    unload_texture(g->assets.mushroom_hit);
    unload_texture(g->assets.mushroom_die);
    unload_texture(g->assets.flying_fly);
    unload_texture(g->assets.flying_hit);
    unload_texture(g->assets.flying_die);
    unload_texture(g->assets.iceball);
    unload_texture(g->assets.start_screen);
    unload_texture(g->assets.defeat_screen);
    g->game_objects = nullptr;
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
int get_game_objects_of_type(const game *g, const object_type type, game_object **out_objects) {
    if (g == nullptr || out_objects == nullptr) {
        return -1;
    }

    int count = 0;
    for (size_t i = 0; i < g->object_count;i++) {
        if (g->game_objects[i].type == type) {
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
    for (size_t i = 0; i < g->object_count; i++) {
        if (g->game_objects[i].type == type) {
            (*out_objects)[current_index] = g->game_objects[i];
            current_index++;
        }
    }

    return count;
}

void update_game_state(game *g, const float delta_time) {
    if (g == nullptr || g->game_objects == nullptr) return;

    for (size_t i = 0; i < g->object_count; i++ ) {
        switch (g->game_objects[i].type) {
            case enemy:
                update_enemy(&g->game_objects[i], delta_time);
                break;
            case tower:
                update_tower(g, &g->game_objects[i], delta_time);
                break;
            case projectile:
                update_projectile(g, &g->game_objects[i], delta_time);
                break;
            default:
                break;
        }
    }

    remove_inactive_objects(g);
}

int find_tower_spot_at_grid(const game *g, const grid_coord coord) {
    if (g == nullptr) return -1;

    for (int i = 0; i < 4; i++) {
        const tower_spot* spot = &g->tower_spots[i];
        const int spot_x = (int)spot->position.x;
        const int spot_y = (int)spot->position.y;

        const bool is_inside = coord.x >= spot_x &&
                               coord.x - spot_x < 4 &&
                               coord.y >= spot_y &&
                               coord.y - spot_y < 4;

        if (is_inside) {
            return i;
        }
    }
    return -1;
}

bool try_build_tower(game *g, const int spot_index) {
    if (g == nullptr) return false;

    if (spot_index < 0 || spot_index >= 4) {
        fprintf(stderr, "ERROR: Invalid tower spot index: %d\n", spot_index);
        return false;
    }

    tower_spot* spot = &g->tower_spots[spot_index];

    if (spot->occupied) {
        return false;
    }

    if (g->player_money < TOWER_BUILD_COST) {
        return false;
    }

    g->player_money -= TOWER_BUILD_COST;
    const result_code res = add_game_object(g, init_tower(spot->position));
    if (res != result_ok) {
        fprintf(stderr, "ERROR: Failed to build tower: code %u\n", (unsigned)res);
        g->player_money += TOWER_BUILD_COST;
        return false;
    }
    spot->occupied = true;

    return true;
}

game_object* find_tower_at_grid(const game *g, const grid_coord coord) {
    if (g == nullptr || g->game_objects == nullptr) return nullptr;

    for (size_t i = 0; i < g->object_count; i++) {
        if (g->game_objects[i].type != tower) {
            continue;
        }

        game_object* twr = &g->game_objects[i];

        const int tower_grid_x = (int)twr->position.x;
        const int tower_grid_y = (int)twr->position.y;
        const int tower_width  = twr->data.tower.width;
        const int tower_height = twr->data.tower.height;

        const bool is_inside = coord.x >= tower_grid_x &&
                               coord.x < tower_grid_x + tower_width &&
                               coord.y >= tower_grid_y &&
                               coord.y < tower_grid_y + tower_height;

        if (is_inside) {
            return twr;
        }
    }
    return nullptr;
}

void remove_inactive_objects(game *g) {
    if (g == nullptr || g->game_objects == nullptr) return;

    size_t write_index = 0;
    for (size_t read_index = 0; read_index < g->object_count; read_index++) {
        const game_object* obj = &g->game_objects[read_index];

        if (!obj->is_active) {
            if (obj->type == enemy) {
                g->enemies_alive--;
                if (obj->data.enemy.health <= 0) {
                    g->enemies_defeated++;
                    g->player_money += obj->data.enemy.gold_reward;
                }
                else {
                    g->player_lives--;
                }
            }
            continue;
        }

        if (write_index != read_index) {
            g->game_objects[write_index] = g->game_objects[read_index];
        }
        write_index++;
    }

    g->object_count = write_index;

    if (g->object_capacity > (size_t)STARTING_COUNT_OF_GAME_OBJECTS * 2 &&
        g->object_count < g->object_capacity / 4) {

        const size_t new_capacity = g->object_capacity / 2;

        game_object* temp = realloc(g->game_objects, sizeof(game_object) * new_capacity);

        if (temp != nullptr) {
            g->game_objects = temp;
            g->object_capacity = new_capacity;
        } else {
            fprintf(stderr, "WARNING: Failed to compact object array\n");
        }
    }
}
