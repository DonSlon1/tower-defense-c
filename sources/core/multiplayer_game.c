#include "multiplayer_game.h"
#include "game.h"
#include "network.h"
#include "multiplayer_ui.h"
#include "renderer.h"
#include "raylib.h"
#include "tower.h"
#include <stdio.h>

// Forward declarations for helper functions from game.c
void handle_playing_input(game *game);
void spawn_enemy(game *game);

void run_multiplayer_host_game(network_state* net, int window_width, int window_height)
{
    set_window_size(window_width, window_height);

    game local_game = init_game();
    game remote_game = init_game();

    start_next_wave(&local_game);
    local_game.state = game_state_playing;
    start_next_wave(&remote_game);
    remote_game.state = game_state_playing;

    multiplayer_ui mp_ui = init_multiplayer_ui(true, window_width, window_height);

    bool local_wave_complete = false;
    bool remote_wave_complete = false;

    float game_sync_timer = 0.0f;

    while (!window_should_close() && network_is_connected(net)) {
        constexpr float game_sync_interval = 1.0f;
        const float delta = get_frame_time();

        const int current_width = get_screen_width();
        const int current_height = get_screen_height();
        update_multiplayer_ui_dimensions(&mp_ui, current_width, current_height);

        if (local_game.state == game_state_playing) {
            if (local_game.player_lives <= 0) {
                local_game.state = game_state_game_over;
            }

            const grid_coord grid_pos = screen_to_grid(get_mouse_position(), &local_game.tilemap);
            const game_object* hovered_tower = find_tower_at_grid(&local_game, grid_pos);
            const int spot_index = find_tower_spot_at_grid(&local_game, grid_pos);

            if (hovered_tower != nullptr || spot_index >= 0) {
                use_pointer_cursor();
            } else {
                use_normal_cursor();
            }

            if (is_mouse_button_pressed(mouse_button_left)) {
                const upgrade_result result = upgrade_clicked_tower(&local_game, grid_pos);
                switch (result) {
                    case upgrade_success:
                        // Send tower upgrade to opponent
                        typedef struct { uint8_t spot; uint8_t level; } tower_upgrade_data;
                        tower_upgrade_data upgrade_data = {
                            .spot = (uint8_t)spot_index,
                            .level = hovered_tower ? hovered_tower->data.tower.level : 0
                        };
                        network_message msg = network_create_message(msg_tower_upgrade, &upgrade_data, sizeof(upgrade_data));
                        network_send(net, &msg);
                        break;
                    case upgrade_insufficient_funds:
                    case upgrade_max_level:
                        break;
                    case upgrade_not_found:
                        if (spot_index >= 0) {
                            if (try_build_tower(&local_game, spot_index)) {
                                // Send tower build to opponent
                                typedef struct { uint8_t spot; } tower_build_data;
                                tower_build_data build_data = { .spot = (uint8_t)spot_index };
                                network_message build_msg = network_create_message(msg_tower_build, &build_data, sizeof(build_data));
                                network_send(net, &build_msg);
                            }
                        }
                        break;
                }
            }

            const wave_config current_wave = get_wave_config(local_game.current_wave);
            if (local_game.enemies_spawned_in_wave < current_wave.enemy_count) {
                local_game.enemy_spawn_timer -= delta;
                if (local_game.enemy_spawn_timer <= 0) {
                    spawn_enemy(&local_game);
                    local_game.enemy_spawn_timer = current_wave.spawn_interval;
                }
            }

            const bool wave_spawning_complete = local_game.enemies_spawned_in_wave >= current_wave.enemy_count;
            if (wave_spawning_complete && local_game.enemies_alive == 0 && !local_wave_complete) {
                local_wave_complete = true;

                // Notify opponent we're done
                network_message msg = network_create_message(msg_wave_complete, nullptr, 0);
                network_send(net, &msg);

                // Check if both players are done
                if (remote_wave_complete) {
                    local_game.state = game_state_wave_break;
                    local_game.wave_break_timer = WAVE_BREAK_DURATION;
                }
            }

            update_game_state(&local_game, delta);
        }

        if (remote_game.state == game_state_playing) {
            const wave_config remote_wave = get_wave_config(remote_game.current_wave);
            if (remote_game.enemies_spawned_in_wave < remote_wave.enemy_count) {
                remote_game.enemy_spawn_timer -= delta;
                if (remote_game.enemy_spawn_timer <= 0) {
                    spawn_enemy(&remote_game);
                    remote_game.enemy_spawn_timer = remote_wave.spawn_interval;
                }
            }

            update_game_state(&remote_game, delta);
        }
        else if (remote_game.state == game_state_wave_break) {
            update_game_state(&remote_game, delta);
            remote_game.wave_break_timer -= delta;
            if (remote_game.wave_break_timer <= 0) {
                start_next_wave(&remote_game);
                remote_game.state = game_state_playing;
            }
        }

        if (local_game.state == game_state_wave_break) {
            handle_playing_input(&local_game);
            update_game_state(&local_game, delta);

            local_game.wave_break_timer -= delta;
            if (local_game.wave_break_timer <= 0 || is_key_pressed(key_space)) {
                start_next_wave(&local_game);
                local_game.state = game_state_playing;

                // Reset wave completion flags
                local_wave_complete = false;
                remote_wave_complete = false;

                // Sync wave start with opponent
                typedef struct { uint8_t wave; } wave_start_data;
                wave_start_data data = { .wave = local_game.current_wave };
                network_message wave_msg = network_create_message(msg_wave_start, &data, sizeof(data));
                network_send(net, &wave_msg);
            }
        }

        update_multiplayer_ui(&mp_ui);

        game_sync_timer += delta;
        if (game_sync_timer >= game_sync_interval) {
            game_sync_timer = 0.0f;

            typedef struct {
                uint16_t money;
                uint16_t lives;
                uint8_t wave;
                uint8_t enemies_alive;
                uint8_t enemies_defeated;
                uint8_t state;  // game_state enum
            } __attribute__((packed)) game_sync_data;

            game_sync_data sync_data = {
                .money = (uint16_t)local_game.player_money,
                .lives = (uint16_t)local_game.player_lives,
                .wave = (uint8_t)local_game.current_wave,
                .enemies_alive = (uint8_t)local_game.enemies_alive,
                .enemies_defeated = (uint8_t)local_game.enemies_defeated,
                .state = (uint8_t)local_game.state
            };

            network_message sync_msg = network_create_message(msg_game_sync, &sync_data, sizeof(sync_data));
            network_send(net, &sync_msg);
        }

        int cost = 0;
        const int enemy_count = check_send_button_clicked(&mp_ui, &cost);
        if (enemy_count > 0 && local_game.player_money >= cost) {
            local_game.player_money -= cost;

            typedef struct { uint8_t count; } send_enemy_data;
            send_enemy_data data = { .count = enemy_count };
            network_message msg = network_create_message(msg_send_enemies, &data, sizeof(data));
            if (network_send(net, &msg)) {
            } else {
                printf("ERROR: Failed to send network message\n");
            }
        }

        network_message msg;
        while (network_receive(net, &msg)) {

            switch (msg.type) {
                case msg_send_enemies: {
                    typedef struct { uint8_t count; } send_enemy_data;
                    auto data = (const send_enemy_data*)msg.data;

                    // Spawn enemies in our local game
                    // NOTE: These are EXTRA enemies, not part of the wave count
                    for (int i = 0; i < data->count; i++) {
                        spawn_enemy(&local_game);
                        // Don't increment enemies_spawned_in_wave - these are bonus enemies!
                    }
                    break;
                }

                case msg_tower_build: {
                    typedef struct { uint8_t spot; } tower_build_data;
                    auto data = (const tower_build_data*)msg.data;

                    // Build tower in remote game
                    if (data->spot < 4) {
                        try_build_tower(&remote_game, data->spot);
                    }
                    break;
                }

                case msg_tower_upgrade: {
                    typedef struct { uint8_t spot; uint8_t level; } tower_upgrade_data;
                    auto data = (const tower_upgrade_data*)msg.data;

                    // Find and upgrade tower in remote game
                    if (data->spot < 4) {
                        const tower_spot* spot = &remote_game.tower_spots[data->spot];
                        game_object* tower_obj = find_tower_at_grid(&remote_game, (grid_coord){
                            .x = (int)spot->position.x,
                            .y = (int)spot->position.y
                        });
                        if (tower_obj) {
                            upgrade_clicked_tower(&remote_game, (grid_coord){
                                .x = (int)tower_obj->position.x,
                                .y = (int)tower_obj->position.y
                            });
                        }
                    }
                    break;
                }

                case msg_wave_complete: {
                    remote_wave_complete = true;

                    // Check if both players are done
                    if (local_wave_complete && local_game.state == game_state_playing) {
                        local_game.state = game_state_wave_break;
                        local_game.wave_break_timer = WAVE_BREAK_DURATION;
                    }
                    break;
                }

                case msg_wave_start: {
                    typedef struct { uint8_t wave; } wave_start_data;
                    auto data = (const wave_start_data*)msg.data;

                    // Sync opponent's game wave
                    if (remote_game.current_wave < data->wave) {
                        remote_game.current_wave = data->wave;
                        remote_game.state = game_state_playing;
                        remote_game.enemies_spawned_in_wave = 0;
                    }

                    // Start local wave too
                    if (local_game.current_wave < data->wave && local_game.state == game_state_wave_break) {
                        start_next_wave(&local_game);
                        local_game.state = game_state_playing;
                        local_wave_complete = false;
                        remote_wave_complete = false;
                    }
                    break;
                }

                case msg_game_sync: {
                    typedef struct {
                        uint16_t money;
                        uint16_t lives;
                        uint8_t wave;
                        uint8_t enemies_alive;
                        uint8_t enemies_defeated;
                        uint8_t state;
                    } __attribute__((packed)) game_sync_data;

                    auto data = (const game_sync_data*)msg.data;

                    // Update remote game state with opponent's data
                    remote_game.player_money = data->money;
                    remote_game.player_lives = data->lives;
                    remote_game.current_wave = data->wave;
                    remote_game.enemies_alive = data->enemies_alive;
                    remote_game.enemies_defeated = data->enemies_defeated;
                    remote_game.state = (game_state)data->state;
                    break;
                }

                case msg_ping: {
                    break;
                }

                default:
                    break;
            }
        }

        begin_drawing();
        clear_background(black);

        draw_tilemap(&local_game.tilemap);
        draw_tower_spots(&local_game);
        draw_game_objects(&local_game);
        draw_hud(&local_game);
        draw_wave_info(&local_game);

        // Show waiting message when local is complete but remote isn't
        if (local_wave_complete && !remote_wave_complete) {
            draw_rectangle(200, 250, 400, 100, (color){0, 0, 0, 200});
            draw_text("WAVE COMPLETE!", 250, 270, 24, green);
            draw_text("Waiting for opponent...", 220, 310, 18, gold);
        }
        else if (local_game.state == game_state_wave_break) {
            char timer_text[64];
            snprintf(timer_text, sizeof(timer_text), "Next wave in: %.0f", local_game.wave_break_timer);
            draw_rectangle(200, 250, 400, 80, (color){0, 0, 0, 200});
            draw_text(timer_text, 260, 280, 24, green);
        }

        render_split_screen(&mp_ui, &local_game, &remote_game);

        // Draw opponent's game (right side) using viewport
        set_viewport(mp_ui.split_x, 0, mp_ui.game_width, mp_ui.game_height);

        draw_tilemap(&remote_game.tilemap);
        draw_tower_spots(&remote_game);
        draw_game_objects(&remote_game);
        draw_hud(&remote_game);
        draw_wave_info(&remote_game);

        reset_viewport();

        render_enemy_send_ui(&mp_ui, &local_game);

        render_connection_status(&mp_ui);

        end_drawing();

        if (is_key_pressed(SDLK_ESCAPE)) {
            break;
        }
    }

    unload_game(&local_game);
    unload_game(&remote_game);
}

void run_multiplayer_client_game(network_state* net, int window_width, int window_height)
{
    set_window_size(window_width, window_height);

    game local_game = init_game();
    game remote_game = init_game();

    start_next_wave(&local_game);
    local_game.state = game_state_playing;
    start_next_wave(&remote_game);
    remote_game.state = game_state_playing;

    multiplayer_ui mp_ui = init_multiplayer_ui(false, window_width, window_height);

    bool local_wave_complete = false;
    bool remote_wave_complete = false;

    float game_sync_timer = 0.0f;

    while (!window_should_close() && network_is_connected(net)) {
        constexpr float game_sync_interval = 1.0f;
        const float delta = get_frame_time();

        const int current_width = get_screen_width();
        const int current_height = get_screen_height();
        update_multiplayer_ui_dimensions(&mp_ui, current_width, current_height);

        if (local_game.state == game_state_playing) {
            if (local_game.player_lives <= 0) {
                local_game.state = game_state_game_over;
            }

            const grid_coord grid_pos = screen_to_grid(get_mouse_position(), &local_game.tilemap);
            const game_object* hovered_tower = find_tower_at_grid(&local_game, grid_pos);
            const int spot_index = find_tower_spot_at_grid(&local_game, grid_pos);

            if (hovered_tower != nullptr || spot_index >= 0) {
                use_pointer_cursor();
            } else {
                use_normal_cursor();
            }

            if (is_mouse_button_pressed(mouse_button_left)) {
                const upgrade_result result = upgrade_clicked_tower(&local_game, grid_pos);
                switch (result) {
                    case upgrade_success:
                        // Send tower upgrade to opponent
                        typedef struct { uint8_t spot; uint8_t level; } tower_upgrade_data;
                        tower_upgrade_data upgrade_data = {
                            .spot = (uint8_t)spot_index,
                            .level = hovered_tower ? hovered_tower->data.tower.level : 0
                        };
                        network_message msg = network_create_message(msg_tower_upgrade, &upgrade_data, sizeof(upgrade_data));
                        network_send(net, &msg);
                        break;
                    case upgrade_insufficient_funds:
                    case upgrade_max_level:
                        break;
                    case upgrade_not_found:
                        if (spot_index >= 0) {
                            if (try_build_tower(&local_game, spot_index)) {
                                // Send tower build to opponent
                                typedef struct { uint8_t spot; } tower_build_data;
                                tower_build_data build_data = { .spot = (uint8_t)spot_index };
                                network_message build_msg = network_create_message(msg_tower_build, &build_data, sizeof(build_data));
                                network_send(net, &build_msg);
                            }
                        }
                        break;
                }
            }

            const wave_config current_wave = get_wave_config(local_game.current_wave);
            if (local_game.enemies_spawned_in_wave < current_wave.enemy_count) {
                local_game.enemy_spawn_timer -= delta;
                if (local_game.enemy_spawn_timer <= 0) {
                    spawn_enemy(&local_game);
                    local_game.enemy_spawn_timer = current_wave.spawn_interval;
                }
            }

            const bool wave_spawning_complete = local_game.enemies_spawned_in_wave >= current_wave.enemy_count;
            if (wave_spawning_complete && local_game.enemies_alive == 0 && !local_wave_complete) {
                local_wave_complete = true;

                // Notify opponent we're done
                network_message msg = network_create_message(msg_wave_complete, nullptr, 0);
                network_send(net, &msg);

                // Check if both players are done
                if (remote_wave_complete) {
                    local_game.state = game_state_wave_break;
                    local_game.wave_break_timer = WAVE_BREAK_DURATION;
                }
            }

            update_game_state(&local_game, delta);
        }

        if (remote_game.state == game_state_playing) {
            const wave_config remote_wave = get_wave_config(remote_game.current_wave);
            if (remote_game.enemies_spawned_in_wave < remote_wave.enemy_count) {
                remote_game.enemy_spawn_timer -= delta;
                if (remote_game.enemy_spawn_timer <= 0) {
                    spawn_enemy(&remote_game);
                    remote_game.enemy_spawn_timer = remote_wave.spawn_interval;
                }
            }

            update_game_state(&remote_game, delta);
        }
        else if (remote_game.state == game_state_wave_break) {
            update_game_state(&remote_game, delta);
            remote_game.wave_break_timer -= delta;
            if (remote_game.wave_break_timer <= 0) {
                start_next_wave(&remote_game);
                remote_game.state = game_state_playing;
            }
        }

        if (local_game.state == game_state_wave_break) {
            handle_playing_input(&local_game);
            update_game_state(&local_game, delta);

            local_game.wave_break_timer -= delta;
            if (local_game.wave_break_timer <= 0 || is_key_pressed(key_space)) {
                start_next_wave(&local_game);
                local_game.state = game_state_playing;

                // Reset wave completion flags
                local_wave_complete = false;
                remote_wave_complete = false;

                // Sync wave start with opponent
                typedef struct { uint8_t wave; } wave_start_data;
                wave_start_data data = { .wave = local_game.current_wave };
                network_message wave_msg = network_create_message(msg_wave_start, &data, sizeof(data));
                network_send(net, &wave_msg);
            }
        }

        update_multiplayer_ui(&mp_ui);

        game_sync_timer += delta;
        if (game_sync_timer >= game_sync_interval) {
            game_sync_timer = 0.0f;

            typedef struct {
                uint16_t money;
                uint16_t lives;
                uint8_t wave;
                uint8_t enemies_alive;
                uint8_t enemies_defeated;
                uint8_t state;  // game_state enum
            } __attribute__((packed)) game_sync_data;

            game_sync_data sync_data = {
                .money = (uint16_t)local_game.player_money,
                .lives = (uint16_t)local_game.player_lives,
                .wave = (uint8_t)local_game.current_wave,
                .enemies_alive = (uint8_t)local_game.enemies_alive,
                .enemies_defeated = (uint8_t)local_game.enemies_defeated,
                .state = (uint8_t)local_game.state
            };

            network_message sync_msg = network_create_message(msg_game_sync, &sync_data, sizeof(sync_data));
            network_send(net, &sync_msg);
        }

        int cost = 0;
        const int enemy_count = check_send_button_clicked(&mp_ui, &cost);
        if (enemy_count > 0 && local_game.player_money >= cost) {
            local_game.player_money -= cost;

            typedef struct { uint8_t count; } send_enemy_data;
            send_enemy_data data = { .count = enemy_count };
            network_message msg = network_create_message(msg_send_enemies, &data, sizeof(data));
            if (network_send(net, &msg)) {
            } else {
                printf("ERROR: Failed to send network message\n");
            }
        }

        network_message msg;
        while (network_receive(net, &msg)) {

            switch (msg.type) {
                case msg_send_enemies: {
                    typedef struct { uint8_t count; } send_enemy_data;
                    auto data = (const send_enemy_data*)msg.data;

                    // Spawn enemies in our local game
                    // NOTE: These are EXTRA enemies, not part of the wave count
                    for (int i = 0; i < data->count; i++) {
                        spawn_enemy(&local_game);
                        // Don't increment enemies_spawned_in_wave - these are bonus enemies!
                    }
                    break;
                }

                case msg_tower_build: {
                    typedef struct { uint8_t spot; } tower_build_data;
                    auto data = (const tower_build_data*)msg.data;

                    // Build tower in remote game
                    if (data->spot < 4) {
                        try_build_tower(&remote_game, data->spot);
                    }
                    break;
                }

                case msg_tower_upgrade: {
                    typedef struct { uint8_t spot; uint8_t level; } tower_upgrade_data;
                    auto data = (const tower_upgrade_data*)msg.data;

                    // Find and upgrade tower in remote game
                    if (data->spot < 4) {
                        const tower_spot* spot = &remote_game.tower_spots[data->spot];
                        game_object* tower_obj = find_tower_at_grid(&remote_game, (grid_coord){
                            .x = (int)spot->position.x,
                            .y = (int)spot->position.y
                        });
                        if (tower_obj) {
                            upgrade_clicked_tower(&remote_game, (grid_coord){
                                .x = (int)tower_obj->position.x,
                                .y = (int)tower_obj->position.y
                            });
                        }
                    }
                    break;
                }

                case msg_wave_complete: {
                    remote_wave_complete = true;

                    // Check if both players are done
                    if (local_wave_complete && local_game.state == game_state_playing) {
                        local_game.state = game_state_wave_break;
                        local_game.wave_break_timer = WAVE_BREAK_DURATION;
                    }
                    break;
                }

                case msg_wave_start: {
                    typedef struct { uint8_t wave; } wave_start_data;
                    auto data = (const wave_start_data*)msg.data;

                    // Sync opponent's game wave
                    if (remote_game.current_wave < data->wave) {
                        remote_game.current_wave = data->wave;
                        remote_game.state = game_state_playing;
                        remote_game.enemies_spawned_in_wave = 0;
                    }

                    // Start local wave too
                    if (local_game.current_wave < data->wave && local_game.state == game_state_wave_break) {
                        start_next_wave(&local_game);
                        local_game.state = game_state_playing;
                        local_wave_complete = false;
                        remote_wave_complete = false;
                    }
                    break;
                }

                case msg_game_sync: {
                    typedef struct {
                        uint16_t money;
                        uint16_t lives;
                        uint8_t wave;
                        uint8_t enemies_alive;
                        uint8_t enemies_defeated;
                        uint8_t state;
                    } __attribute__((packed)) game_sync_data;

                    auto data = (const game_sync_data*)msg.data;

                    // Update remote game state with opponent's data
                    remote_game.player_money = data->money;
                    remote_game.player_lives = data->lives;
                    remote_game.current_wave = data->wave;
                    remote_game.enemies_alive = data->enemies_alive;
                    remote_game.enemies_defeated = data->enemies_defeated;
                    remote_game.state = (game_state)data->state;
                    break;
                }

                case msg_ping: {
                    break;
                }

                default:
                    break;
            }
        }

        begin_drawing();
        clear_background(black);

        draw_tilemap(&local_game.tilemap);
        draw_tower_spots(&local_game);
        draw_game_objects(&local_game);
        draw_hud(&local_game);
        draw_wave_info(&local_game);

        // Show waiting message when local is complete but remote isn't
        if (local_wave_complete && !remote_wave_complete) {
            draw_rectangle(200, 250, 400, 100, (color){0, 0, 0, 200});
            draw_text("WAVE COMPLETE!", 250, 270, 24, green);
            draw_text("Waiting for opponent...", 220, 310, 18, gold);
        }
        else if (local_game.state == game_state_wave_break) {
            char timer_text[64];
            snprintf(timer_text, sizeof(timer_text), "Next wave in: %.0f", local_game.wave_break_timer);
            draw_rectangle(200, 250, 400, 80, (color){0, 0, 0, 200});
            draw_text(timer_text, 260, 280, 24, green);
        }

        render_split_screen(&mp_ui, &local_game, &remote_game);

        // Draw opponent's game (right side) using viewport
        set_viewport(mp_ui.split_x, 0, mp_ui.game_width, mp_ui.game_height);

        draw_tilemap(&remote_game.tilemap);
        draw_tower_spots(&remote_game);
        draw_game_objects(&remote_game);
        draw_hud(&remote_game);
        draw_wave_info(&remote_game);

        reset_viewport();

        render_enemy_send_ui(&mp_ui, &local_game);

        render_connection_status(&mp_ui);

        end_drawing();

        if (is_key_pressed(SDLK_ESCAPE)) {
            break;
        }
    }

    unload_game(&local_game);
    unload_game(&remote_game);
}
