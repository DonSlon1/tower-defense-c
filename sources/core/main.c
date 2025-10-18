#include "game.h"
#include "renderer.h"
#include "raylib.h"
#include "menu.h"
#include "network.h"
#include "multiplayer_ui.h"
#include "tower.h"

// Forward declarations for game functions
void handle_playing_input(game *game);
void spawn_enemy(game *game);

// Helper function to render game with X offset (for split-screen)
void render_game_with_offset(const game* g, int offset_x) {
    // TODO: This is a simplified version - proper offset rendering requires
    // modifying all draw functions to support viewport translation
    // For now, we just show basic info - full rendering in Phase 6

    // We can't easily offset the existing rendering functions without major refactoring
    // So for now, show text-based representation
}

// Screen dimensions
#define MENU_WIDTH 800
#define MENU_HEIGHT 600
#define MULTIPLAYER_WIDTH 1600
#define MULTIPLAYER_HEIGHT 600

// Window title
#define WINDOW_TITLE "Tower Defense"

int main(void)
{
    // Start with menu-sized window
    init_window(MENU_WIDTH, MENU_HEIGHT, WINDOW_TITLE);
    set_target_fps(60);
    set_window_icon(ASSETS_PATH "images/towers.png");
    set_mouse_cursor(ASSETS_PATH "cursor/Middle Ages--cursor--SweezyCursors.png");
    set_mouse_pointer(ASSETS_PATH "cursor/Middle Ages--pointer--SweezyCursors.png");

    // Initialize menu system
    menu_system menu = init_menu_system();

    // Track if we've already attempted multiplayer connection
    bool connection_attempted = false;
    menu_state last_menu_state = MENU_STATE_MAIN;

    // Network state persists across frames
    network_state* active_network = NULL;

    // Main loop - menu mode
    while (!window_should_close()) {
        // Reset connection attempt flag if menu state changed
        if (menu.current_state != last_menu_state) {
            printf("DEBUG: Menu state changed from %d to %d\n", last_menu_state, menu.current_state);
            connection_attempted = false;
            last_menu_state = menu.current_state;
        }

        // If we have an active network connection waiting (host), check for client
        if (active_network && menu_is_host(&menu) && !network_is_connected(active_network)) {
            if (network_host_check_for_client(active_network)) {
                printf("SUCCESS: Client connected!\n");
                printf("Starting multiplayer game...\n");
                menu_set_player_connected(&menu);

                // Resize window for split-screen
                set_window_size(MULTIPLAYER_WIDTH, MULTIPLAYER_HEIGHT);

                // Initialize both game states
                game local_game = init_game();
                game remote_game = init_game();

                // Start both games (wave 1)
                start_next_wave(&local_game);
                local_game.state = game_state_playing;
                start_next_wave(&remote_game);
                remote_game.state = game_state_playing;

                // Initialize multiplayer UI
                multiplayer_ui mp_ui = init_multiplayer_ui(true, MULTIPLAYER_WIDTH, MULTIPLAYER_HEIGHT);

                // Wave synchronization flags
                bool local_wave_complete = false;
                bool remote_wave_complete = false;

                // Game sync timer (send state every second)
                float game_sync_timer = 0.0f;
                const float GAME_SYNC_INTERVAL = 1.0f;

                // Multiplayer game loop
                while (!window_should_close() && network_is_connected(active_network)) {
                    const float delta = get_frame_time();

                    // Update UI dimensions based on current window size
                    const int current_width = get_screen_width();
                    const int current_height = get_screen_height();
                    update_multiplayer_ui_dimensions(&mp_ui, current_width, current_height);

                    // Handle game logic for local game
                    if (local_game.state == game_state_playing) {
                        if (local_game.player_lives <= 0) {
                            local_game.state = game_state_game_over;
                        }

                        // Handle input (tower building, upgrading) with network sync
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
                                    printf("Tower upgraded successfully!\n");
                                    // Send tower upgrade to opponent
                                    typedef struct { uint8_t spot; uint8_t level; } tower_upgrade_data;
                                    tower_upgrade_data upgrade_data = {
                                        .spot = (uint8_t)spot_index,
                                        .level = hovered_tower ? hovered_tower->data.tower.level : 0
                                    };
                                    network_message msg = network_create_message(MSG_TOWER_UPGRADE, &upgrade_data, sizeof(upgrade_data));
                                    network_send(active_network, &msg);
                                    break;
                                case upgrade_insufficient_funds:
                                    printf("Insufficient funds for upgrade\n");
                                    break;
                                case upgrade_max_level:
                                    printf("Tower is already at max level\n");
                                    break;
                                case upgrade_not_found:
                                    if (spot_index >= 0) {
                                        if (try_build_tower(&local_game, spot_index)) {
                                            // Send tower build to opponent
                                            typedef struct { uint8_t spot; } tower_build_data;
                                            tower_build_data build_data = { .spot = (uint8_t)spot_index };
                                            network_message build_msg = network_create_message(MSG_TOWER_BUILD, &build_data, sizeof(build_data));
                                            network_send(active_network, &build_msg);
                                        }
                                    }
                                    break;
                            }
                        }

                        // Spawn wave enemies automatically (normal wave progression)
                        const wave_config current_wave = get_wave_config(local_game.current_wave);
                        if (local_game.enemies_spawned_in_wave < current_wave.enemy_count) {
                            local_game.enemy_spawn_timer -= delta;
                            if (local_game.enemy_spawn_timer <= 0) {
                                spawn_enemy(&local_game);
                                local_game.enemy_spawn_timer = current_wave.spawn_interval;
                            }
                        }

                        // Check if wave enemies are spawned AND all enemies are dead
                        const bool wave_spawning_complete = local_game.enemies_spawned_in_wave >= current_wave.enemy_count;
                        if (wave_spawning_complete && local_game.enemies_alive == 0 && !local_wave_complete) {
                            local_wave_complete = true;
                            printf("Local wave complete! Waiting for opponent...\n");

                            // Notify opponent we're done
                            network_message msg = network_create_message(MSG_WAVE_COMPLETE, nullptr, 0);
                            network_send(active_network, &msg);

                            // Check if both players are done
                            if (remote_wave_complete) {
                                printf("Both players complete! Starting wave break...\n");
                                local_game.state = game_state_wave_break;
                                local_game.wave_break_timer = WAVE_BREAK_DURATION;
                            }
                        }

                        // Update game state (move enemies, towers shoot, etc.)
                        update_game_state(&local_game, delta);
                    }

                    // Always update remote game (so opponent's game runs independently)
                    if (remote_game.state == game_state_playing) {
                        // Spawn wave enemies automatically for remote game
                        const wave_config remote_wave = get_wave_config(remote_game.current_wave);
                        if (remote_game.enemies_spawned_in_wave < remote_wave.enemy_count) {
                            remote_game.enemy_spawn_timer -= delta;
                            if (remote_game.enemy_spawn_timer <= 0) {
                                spawn_enemy(&remote_game);
                                remote_game.enemy_spawn_timer = remote_wave.spawn_interval;
                            }
                        }

                        update_game_state(&remote_game, delta);

                        // Debug: Print remote game enemy count every 2 seconds
                        static float debug_timer = 0;
                        debug_timer += delta;
                        if (debug_timer >= 2.0f) {
                            printf("[DEBUG] Remote game: %d enemies, %d objects total, spawned: %d/%d\n",
                                   remote_game.enemies_alive, (int)remote_game.object_count,
                                   remote_game.enemies_spawned_in_wave, remote_wave.enemy_count);
                            debug_timer = 0;
                        }
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
                            network_message wave_msg = network_create_message(MSG_WAVE_START, &data, sizeof(data));
                            network_send(active_network, &wave_msg);
                            printf("Started wave %d, syncing with opponent\n", local_game.current_wave);
                        }
                    }

                    // Update multiplayer UI
                    update_multiplayer_ui(&mp_ui);

                    // Send periodic game state sync
                    game_sync_timer += delta;
                    if (game_sync_timer >= GAME_SYNC_INTERVAL) {
                        game_sync_timer = 0.0f;

                        // Pack essential game state into a compact message
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

                        network_message sync_msg = network_create_message(MSG_GAME_SYNC, &sync_data, sizeof(sync_data));
                        if (network_send(active_network, &sync_msg)) {
                            printf("[SYNC] Sent game state: money=%d lives=%d wave=%d enemies=%d\n",
                                   sync_data.money, sync_data.lives, sync_data.wave, sync_data.enemies_alive);
                        }
                    }

                    // Check for enemy send button clicks
                    int cost = 0;
                    const int enemy_count = check_send_button_clicked(&mp_ui, &cost);
                    if (enemy_count > 0 && local_game.player_money >= cost) {
                        printf("Sending %d enemies to opponent (cost: $%d)\n", enemy_count, cost);
                        local_game.player_money -= cost;

                        // Send network message
                        typedef struct { uint8_t count; } send_enemy_data;
                        send_enemy_data data = { .count = enemy_count };
                        network_message msg = network_create_message(MSG_SEND_ENEMIES, &data, sizeof(data));
                        if (network_send(active_network, &msg)) {
                            printf("Network message sent successfully\n");
                        } else {
                            printf("ERROR: Failed to send network message\n");
                        }
                    }

                    // Receive and process network messages
                    network_message msg;
                    while (network_receive(active_network, &msg)) {
                        printf("Received network message type %d\n", msg.type);

                        switch (msg.type) {
                            case MSG_SEND_ENEMIES: {
                                typedef struct { uint8_t count; } send_enemy_data;
                                const send_enemy_data* data = (const send_enemy_data*)msg.data;
                                printf("Opponent sent %d EXTRA enemies to us!\n", data->count);

                                // Spawn enemies in our local game
                                // NOTE: These are EXTRA enemies, not part of the wave count
                                for (int i = 0; i < data->count; i++) {
                                    spawn_enemy(&local_game);
                                    // Don't increment enemies_spawned_in_wave - these are bonus enemies!
                                }
                                break;
                            }

                            case MSG_TOWER_BUILD: {
                                typedef struct { uint8_t spot; } tower_build_data;
                                const tower_build_data* data = (const tower_build_data*)msg.data;
                                printf("Opponent built a tower at spot %d\n", data->spot);

                                // Build tower in remote game
                                if (data->spot < 4) {
                                    try_build_tower(&remote_game, data->spot);
                                }
                                break;
                            }

                            case MSG_TOWER_UPGRADE: {
                                typedef struct { uint8_t spot; uint8_t level; } tower_upgrade_data;
                                const tower_upgrade_data* data = (const tower_upgrade_data*)msg.data;
                                printf("Opponent upgraded tower at spot %d to level %d\n", data->spot, data->level);

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

                            case MSG_WAVE_COMPLETE: {
                                printf("Opponent completed their wave!\n");
                                remote_wave_complete = true;

                                // Check if both players are done
                                if (local_wave_complete && local_game.state == game_state_playing) {
                                    printf("Both players complete! Starting wave break...\n");
                                    local_game.state = game_state_wave_break;
                                    local_game.wave_break_timer = WAVE_BREAK_DURATION;
                                }
                                break;
                            }

                            case MSG_WAVE_START: {
                                typedef struct { uint8_t wave; } wave_start_data;
                                const wave_start_data* data = (const wave_start_data*)msg.data;
                                printf("Opponent started wave %d\n", data->wave);

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

                            case MSG_GAME_SYNC: {
                                typedef struct {
                                    uint16_t money;
                                    uint16_t lives;
                                    uint8_t wave;
                                    uint8_t enemies_alive;
                                    uint8_t enemies_defeated;
                                    uint8_t state;
                                } __attribute__((packed)) game_sync_data;

                                const game_sync_data* data = (const game_sync_data*)msg.data;

                                printf("[SYNC] Received opponent state: money=%d lives=%d wave=%d enemies=%d\n",
                                       data->money, data->lives, data->wave, data->enemies_alive);

                                // Update remote game state with opponent's data
                                remote_game.player_money = data->money;
                                remote_game.player_lives = data->lives;
                                remote_game.current_wave = data->wave;
                                remote_game.enemies_alive = data->enemies_alive;
                                remote_game.enemies_defeated = data->enemies_defeated;
                                remote_game.state = (game_state)data->state;
                                break;
                            }

                            case MSG_PING: {
                                printf("Received ping from opponent\n");
                                break;
                            }

                            default:
                                printf("Unknown message type: %d\n", msg.type);
                                break;
                        }
                    }

                    // Render
                    begin_drawing();
                    clear_background(black);

                    // Draw local game (left side)
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
                    // Show wave break countdown when both players are done
                    else if (local_game.state == game_state_wave_break) {
                        char timer_text[64];
                        snprintf(timer_text, sizeof(timer_text), "Next wave in: %.0f", local_game.wave_break_timer);
                        draw_rectangle(200, 250, 400, 80, (color){0, 0, 0, 200});
                        draw_text(timer_text, 260, 280, 24, green);
                    }

                    // Draw split screen UI elements (divider and labels)
                    render_split_screen(&mp_ui, &local_game, &remote_game);

                    // Draw opponent's game (right side) using viewport
                    set_viewport(mp_ui.split_x, 0, mp_ui.game_width, mp_ui.game_height);

                    // Render opponent's game (same as local game rendering)
                    draw_tilemap(&remote_game.tilemap);
                    draw_tower_spots(&remote_game);
                    draw_game_objects(&remote_game);
                    draw_hud(&remote_game);
                    draw_wave_info(&remote_game);

                    // Reset viewport
                    reset_viewport();

                    // Render enemy send UI
                    render_enemy_send_ui(&mp_ui, &local_game);

                    // Render connection status
                    render_connection_status(&mp_ui);

                    end_drawing();

                    // ESC to quit
                    if (is_key_pressed(SDLK_ESCAPE)) {
                        printf("User quit multiplayer game\n");
                        break;
                    }
                }

                // Cleanup
                unload_game(&local_game);
                unload_game(&remote_game);
                network_close(active_network);
                active_network = NULL;

                // Resize back to menu size
                set_window_size(MENU_WIDTH, MENU_HEIGHT);
                menu.current_state = MENU_STATE_MAIN;
                connection_attempted = false;
            }
        }

        // Update menu
        update_menu(&menu);

        // Check if user selected single player
        if (menu_should_start_singleplayer(&menu)) {
            printf("Starting single player game...\n");

            // Create and start game (start_game contains its own game loop)
            game current_game = init_game();
            start_game(&current_game);  // This function runs until user quits or returns to menu
            unload_game(&current_game);

            // Return to menu after game ends
            menu.current_state = MENU_STATE_MAIN;
        }

        // Check if user selected multiplayer (only attempt once per state)
        else if (menu_should_start_multiplayer(&menu) && !connection_attempted && !active_network) {
            connection_attempted = true;  // Mark as attempted
            printf("Starting multiplayer game...\n");

            const bool is_host = menu_is_host(&menu);
            const char* ip = menu_get_ip(&menu);
            const int port = menu_get_port(&menu);

            if (is_host) {
                printf("=== HOST MODE ===\n");
                printf("Hosting on port %d...\n", port);
                fflush(stdout);

                active_network = network_create_host(port);

                if (active_network) {
                    printf("Server socket created, listening for connections...\n");
                    printf("Window will stay responsive, waiting for client...\n");
                    fflush(stdout);

                    // active_network is now set, we'll check for connections each frame
                    // The menu will continue showing "Waiting for player..."
                } else {
                    printf("ERROR: Failed to create server socket\n");
                    menu_set_connection_failed(&menu, "Failed to start server");
                    active_network = NULL;
                }
            } else {
                printf("Connecting to %s:%d...\n", ip, port);
                active_network = network_connect(ip, port);
                if (active_network) {
                    printf("SUCCESS: Connected to server!\n");
                    printf("Starting multiplayer game...\n");
                    menu_set_connection_success(&menu);

                    // Resize window for split-screen
                    set_window_size(MULTIPLAYER_WIDTH, MULTIPLAYER_HEIGHT);

                    // Initialize both game states
                    game local_game = init_game();
                    game remote_game = init_game();

                    // Start both games (wave 1)
                    start_next_wave(&local_game);
                    local_game.state = game_state_playing;
                    start_next_wave(&remote_game);
                    remote_game.state = game_state_playing;

                    // Initialize multiplayer UI
                    multiplayer_ui mp_ui = init_multiplayer_ui(false, MULTIPLAYER_WIDTH, MULTIPLAYER_HEIGHT);

                    // Wave synchronization flags
                    bool local_wave_complete = false;
                    bool remote_wave_complete = false;

                    // Game sync timer (send state every second)
                    float game_sync_timer = 0.0f;
                    const float GAME_SYNC_INTERVAL = 1.0f;

                    // Multiplayer game loop
                    while (!window_should_close() && network_is_connected(active_network)) {
                        const float delta = get_frame_time();

                        // Update UI dimensions based on current window size
                        const int current_width = get_screen_width();
                        const int current_height = get_screen_height();
                        update_multiplayer_ui_dimensions(&mp_ui, current_width, current_height);

                        // Handle game logic for local game
                        if (local_game.state == game_state_playing) {
                            if (local_game.player_lives <= 0) {
                                local_game.state = game_state_game_over;
                            }

                            // Handle input (tower building, upgrading) with network sync
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
                                        printf("Tower upgraded successfully!\n");
                                        // Send tower upgrade to opponent
                                        typedef struct { uint8_t spot; uint8_t level; } tower_upgrade_data;
                                        tower_upgrade_data upgrade_data = {
                                            .spot = (uint8_t)spot_index,
                                            .level = hovered_tower ? hovered_tower->data.tower.level : 0
                                        };
                                        network_message msg = network_create_message(MSG_TOWER_UPGRADE, &upgrade_data, sizeof(upgrade_data));
                                        network_send(active_network, &msg);
                                        break;
                                    case upgrade_insufficient_funds:
                                        printf("Insufficient funds for upgrade\n");
                                        break;
                                    case upgrade_max_level:
                                        printf("Tower is already at max level\n");
                                        break;
                                    case upgrade_not_found:
                                        if (spot_index >= 0) {
                                            if (try_build_tower(&local_game, spot_index)) {
                                                // Send tower build to opponent
                                                typedef struct { uint8_t spot; } tower_build_data;
                                                tower_build_data build_data = { .spot = (uint8_t)spot_index };
                                                network_message build_msg = network_create_message(MSG_TOWER_BUILD, &build_data, sizeof(build_data));
                                                network_send(active_network, &build_msg);
                                            }
                                        }
                                        break;
                                }
                            }

                            // Spawn wave enemies automatically (normal wave progression)
                            const wave_config current_wave = get_wave_config(local_game.current_wave);
                            if (local_game.enemies_spawned_in_wave < current_wave.enemy_count) {
                                local_game.enemy_spawn_timer -= delta;
                                if (local_game.enemy_spawn_timer <= 0) {
                                    spawn_enemy(&local_game);
                                    local_game.enemy_spawn_timer = current_wave.spawn_interval;
                                }
                            }

                            // Check if wave enemies are spawned AND all enemies are dead
                            const bool wave_spawning_complete = local_game.enemies_spawned_in_wave >= current_wave.enemy_count;
                            if (wave_spawning_complete && local_game.enemies_alive == 0 && !local_wave_complete) {
                                local_wave_complete = true;
                                printf("Local wave complete! Waiting for opponent...\n");

                                // Notify opponent we're done
                                network_message msg = network_create_message(MSG_WAVE_COMPLETE, nullptr, 0);
                                network_send(active_network, &msg);

                                // Check if both players are done
                                if (remote_wave_complete) {
                                    printf("Both players complete! Starting wave break...\n");
                                    local_game.state = game_state_wave_break;
                                    local_game.wave_break_timer = WAVE_BREAK_DURATION;
                                }
                            }

                            // Update game state (move enemies, towers shoot, etc.)
                            update_game_state(&local_game, delta);
                        }

                        // Always update remote game (so opponent's game runs independently)
                        if (remote_game.state == game_state_playing) {
                            // Spawn wave enemies automatically for remote game
                            const wave_config remote_wave = get_wave_config(remote_game.current_wave);
                            if (remote_game.enemies_spawned_in_wave < remote_wave.enemy_count) {
                                remote_game.enemy_spawn_timer -= delta;
                                if (remote_game.enemy_spawn_timer <= 0) {
                                    spawn_enemy(&remote_game);
                                    remote_game.enemy_spawn_timer = remote_wave.spawn_interval;
                                }
                            }

                            update_game_state(&remote_game, delta);

                            // Debug: Print remote game enemy count every 2 seconds
                            static float debug_timer = 0;
                            debug_timer += delta;
                            if (debug_timer >= 2.0f) {
                                printf("[DEBUG] Remote game: %d enemies, %d objects total, spawned: %d/%d\n",
                                       remote_game.enemies_alive, (int)remote_game.object_count,
                                       remote_game.enemies_spawned_in_wave, remote_wave.enemy_count);
                                debug_timer = 0;
                            }
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
                                network_message wave_msg = network_create_message(MSG_WAVE_START, &data, sizeof(data));
                                network_send(active_network, &wave_msg);
                                printf("Started wave %d, syncing with opponent\n", local_game.current_wave);
                            }
                        }

                        // Update multiplayer UI
                        update_multiplayer_ui(&mp_ui);

                        // Send periodic game state sync
                        game_sync_timer += delta;
                        if (game_sync_timer >= GAME_SYNC_INTERVAL) {
                            game_sync_timer = 0.0f;

                            // Pack essential game state into a compact message
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

                            network_message sync_msg = network_create_message(MSG_GAME_SYNC, &sync_data, sizeof(sync_data));
                            if (network_send(active_network, &sync_msg)) {
                                printf("[SYNC] Sent game state: money=%d lives=%d wave=%d enemies=%d\n",
                                       sync_data.money, sync_data.lives, sync_data.wave, sync_data.enemies_alive);
                            }
                        }

                        // Check for enemy send button clicks
                        int cost = 0;
                        const int enemy_count = check_send_button_clicked(&mp_ui, &cost);
                        if (enemy_count > 0 && local_game.player_money >= cost) {
                            printf("Sending %d enemies to opponent (cost: $%d)\n", enemy_count, cost);
                            local_game.player_money -= cost;

                            // Send network message
                            typedef struct { uint8_t count; } send_enemy_data;
                            send_enemy_data data = { .count = enemy_count };
                            network_message msg = network_create_message(MSG_SEND_ENEMIES, &data, sizeof(data));
                            if (network_send(active_network, &msg)) {
                                printf("Network message sent successfully\n");
                            } else {
                                printf("ERROR: Failed to send network message\n");
                            }
                        }

                        // Receive and process network messages
                        network_message msg;
                        while (network_receive(active_network, &msg)) {
                            printf("Received network message type %d\n", msg.type);

                            switch (msg.type) {
                                case MSG_SEND_ENEMIES: {
                                    typedef struct { uint8_t count; } send_enemy_data;
                                    const send_enemy_data* data = (const send_enemy_data*)msg.data;
                                    printf("Opponent sent %d EXTRA enemies to us!\n", data->count);

                                    // Spawn enemies in our local game
                                    // NOTE: These are EXTRA enemies, not part of the wave count
                                    for (int i = 0; i < data->count; i++) {
                                        spawn_enemy(&local_game);
                                        // Don't increment enemies_spawned_in_wave - these are bonus enemies!
                                    }
                                    break;
                                }

                                case MSG_TOWER_BUILD: {
                                    typedef struct { uint8_t spot; } tower_build_data;
                                    const tower_build_data* data = (const tower_build_data*)msg.data;
                                    printf("Opponent built a tower at spot %d\n", data->spot);

                                    // Build tower in remote game
                                    if (data->spot < 4) {
                                        try_build_tower(&remote_game, data->spot);
                                    }
                                    break;
                                }

                                case MSG_TOWER_UPGRADE: {
                                    typedef struct { uint8_t spot; uint8_t level; } tower_upgrade_data;
                                    const tower_upgrade_data* data = (const tower_upgrade_data*)msg.data;
                                    printf("Opponent upgraded tower at spot %d to level %d\n", data->spot, data->level);

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

                                case MSG_WAVE_COMPLETE: {
                                    printf("Opponent completed their wave!\n");
                                    remote_wave_complete = true;

                                    // Check if both players are done
                                    if (local_wave_complete && local_game.state == game_state_playing) {
                                        printf("Both players complete! Starting wave break...\n");
                                        local_game.state = game_state_wave_break;
                                        local_game.wave_break_timer = WAVE_BREAK_DURATION;
                                    }
                                    break;
                                }

                                case MSG_WAVE_START: {
                                    typedef struct { uint8_t wave; } wave_start_data;
                                    const wave_start_data* data = (const wave_start_data*)msg.data;
                                    printf("Opponent started wave %d\n", data->wave);

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

                                case MSG_GAME_SYNC: {
                                    typedef struct {
                                        uint16_t money;
                                        uint16_t lives;
                                        uint8_t wave;
                                        uint8_t enemies_alive;
                                        uint8_t enemies_defeated;
                                        uint8_t state;
                                    } __attribute__((packed)) game_sync_data;

                                    const game_sync_data* data = (const game_sync_data*)msg.data;

                                    printf("[SYNC] Received opponent state: money=%d lives=%d wave=%d enemies=%d\n",
                                           data->money, data->lives, data->wave, data->enemies_alive);

                                    // Update remote game state with opponent's data
                                    remote_game.player_money = data->money;
                                    remote_game.player_lives = data->lives;
                                    remote_game.current_wave = data->wave;
                                    remote_game.enemies_alive = data->enemies_alive;
                                    remote_game.enemies_defeated = data->enemies_defeated;
                                    remote_game.state = (game_state)data->state;
                                    break;
                                }

                                case MSG_PING: {
                                    printf("Received ping from opponent\n");
                                    break;
                                }

                                default:
                                    printf("Unknown message type: %d\n", msg.type);
                                    break;
                            }
                        }

                        // Render
                        begin_drawing();
                        clear_background(black);

                        // Draw local game (left side)
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
                        // Show wave break countdown when both players are done
                        else if (local_game.state == game_state_wave_break) {
                            char timer_text[64];
                            snprintf(timer_text, sizeof(timer_text), "Next wave in: %.0f", local_game.wave_break_timer);
                            draw_rectangle(200, 250, 400, 80, (color){0, 0, 0, 200});
                            draw_text(timer_text, 260, 280, 24, green);
                        }

                        // Draw split screen UI elements (divider and labels)
                        render_split_screen(&mp_ui, &local_game, &remote_game);

                        // Draw opponent's game (right side) using viewport
                        set_viewport(mp_ui.split_x, 0, mp_ui.game_width, mp_ui.game_height);

                        // Render opponent's game (same as local game rendering)
                        draw_tilemap(&remote_game.tilemap);
                        draw_tower_spots(&remote_game);
                        draw_game_objects(&remote_game);
                        draw_hud(&remote_game);
                        draw_wave_info(&remote_game);

                        // Reset viewport
                        reset_viewport();

                        // Render enemy send UI
                        render_enemy_send_ui(&mp_ui, &local_game);

                        // Render connection status
                        render_connection_status(&mp_ui);

                        end_drawing();

                        // ESC to quit
                        if (is_key_pressed(SDLK_ESCAPE)) {
                            printf("User quit multiplayer game\n");
                            break;
                        }
                    }

                    // Cleanup
                    unload_game(&local_game);
                    unload_game(&remote_game);
                    network_close(active_network);
                    active_network = NULL;

                    // Resize back to menu size
                    set_window_size(MENU_WIDTH, MENU_HEIGHT);
                    menu.current_state = MENU_STATE_MAIN;
                } else {
                    printf("ERROR: Failed to connect to %s:%d\n", ip, port);
                    menu_set_connection_failed(&menu, "Failed to connect to host");
                    active_network = NULL;
                }
            }
        }

        // Check if user wants to quit
        else if (menu_should_quit(&menu)) {
            break;
        }

        // Render menu
        begin_drawing();
        render_menu(&menu);
        end_drawing();
    }

    // Cleanup
    if (active_network) {
        network_close(active_network);
    }
    cleanup_menu(&menu);
    close_window();

    return 0;
}
