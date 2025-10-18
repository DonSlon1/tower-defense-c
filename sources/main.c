#include "game.h"
#include "raylib.h"
#include "menu.h"
#include "network.h"

// Screen dimensions - 800x600 for menu
#define MENU_WIDTH 800
#define MENU_HEIGHT 600

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

    // Main loop - menu mode
    while (!window_should_close()) {
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

        // Check if user selected multiplayer
        else if (menu_should_start_multiplayer(&menu)) {
            printf("Starting multiplayer game...\n");

            const bool is_host = menu_is_host(&menu);
            const char* ip = menu_get_ip(&menu);
            const int port = menu_get_port(&menu);

            network_state* net = NULL;

            if (is_host) {
                printf("Hosting on port %d...\n", port);
                net = network_create_host(port);
                if (net) {
                    printf("SUCCESS: Player connected!\n");
                    menu_set_player_connected(&menu);

                    // For now, just show success message
                    printf("Connection established! (Game not implemented yet)\n");
                    printf("Press any key to return to menu...\n");

                    // Wait for any key
                    while (!window_should_close() && !is_key_pressed(SDLK_SPACE)) {
                        begin_drawing();
                        clear_background(black);
                        draw_text("CONNECTION SUCCESSFUL!", 200, 250, 30, green);
                        draw_text("Multiplayer game not yet implemented", 150, 300, 20, white);
                        draw_text("Press SPACE to return to menu", 200, 350, 20, lightgray);
                        end_drawing();
                    }

                    network_close(net);
                    menu.current_state = MENU_STATE_MAIN;
                } else {
                    printf("ERROR: Failed to start server\n");
                    menu_set_connection_failed(&menu, "Failed to start server");
                }
            } else {
                printf("Connecting to %s:%d...\n", ip, port);
                net = network_connect(ip, port);
                if (net) {
                    printf("SUCCESS: Connected to server!\n");
                    menu_set_connection_success(&menu);

                    // For now, just show success message
                    printf("Connection established! (Game not implemented yet)\n");
                    printf("Press any key to return to menu...\n");

                    // Wait for any key
                    while (!window_should_close() && !is_key_pressed(SDLK_SPACE)) {
                        begin_drawing();
                        clear_background(black);
                        draw_text("CONNECTION SUCCESSFUL!", 200, 250, 30, green);
                        draw_text("Multiplayer game not yet implemented", 150, 300, 20, white);
                        draw_text("Press SPACE to return to menu", 200, 350, 20, lightgray);
                        end_drawing();
                    }

                    network_close(net);
                    menu.current_state = MENU_STATE_MAIN;
                } else {
                    printf("ERROR: Failed to connect to %s:%d\n", ip, port);
                    menu_set_connection_failed(&menu, "Failed to connect to host");
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
    cleanup_menu(&menu);
    close_window();

    return 0;
}
