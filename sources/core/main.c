#include "game.h"
#include "raylib.h"
#include "menu.h"
#include "network.h"
#include "multiplayer_game.h"

#define MENU_WIDTH 800
#define MENU_HEIGHT 600
#define MULTIPLAYER_WIDTH 1600
#define MULTIPLAYER_HEIGHT 600
#define WINDOW_TITLE "Tower Defense"

int main(void)
{
    init_window(MENU_WIDTH, MENU_HEIGHT, WINDOW_TITLE);
    set_target_fps(60);
    set_window_icon(ASSETS_PATH "images/towers.png");
    set_mouse_cursor(ASSETS_PATH "cursor/Middle Ages--cursor--SweezyCursors.png");
    set_mouse_pointer(ASSETS_PATH "cursor/Middle Ages--pointer--SweezyCursors.png");

    menu_system menu = init_menu_system();
    bool connection_attempted = false;
    menu_state last_menu_state = menu_state_main;
    network_state* active_network = nullptr;

    while (!window_should_close()) {
        if (menu.current_state != last_menu_state) {
            connection_attempted = false;
            last_menu_state = menu.current_state;
        }

        if (active_network && menu_is_host(&menu) && !network_is_connected(active_network)) {
            if (network_host_check_for_client(active_network)) {
                printf("SUCCESS: Client connected!\n");
                printf("Starting multiplayer game...\n");
                menu_set_player_connected(&menu);

                run_multiplayer_host_game(active_network, MULTIPLAYER_WIDTH, MULTIPLAYER_HEIGHT);

                network_close(active_network);
                active_network = nullptr;

                // Resize back to menu size
                set_window_size(MENU_WIDTH, MENU_HEIGHT);
                menu.current_state = menu_state_main;
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
            menu.current_state = menu_state_main;
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

                active_network = network_create_host((uint16_t)port);

                if (active_network) {
                    printf("Server socket created, listening for connections...\n");
                    printf("Window will stay responsive, waiting for client...\n");
                    fflush(stdout);
                } else {
                    printf("ERROR: Failed to create server socket\n");
                    menu_set_connection_failed(&menu, "Port already in use");
                    active_network = nullptr;
                }
            } else {
                printf("Connecting to %s:%d...\n", ip, port);
                active_network = network_connect(ip, (uint16_t)port);
                if (active_network) {
                    printf("SUCCESS: Connected to server!\n");
                    printf("Starting multiplayer game...\n");
                    menu_set_connection_success(&menu);

                    run_multiplayer_client_game(active_network, MULTIPLAYER_WIDTH, MULTIPLAYER_HEIGHT);

                    network_close(active_network);
                    active_network = nullptr;

                    // Resize back to menu size
                    set_window_size(MENU_WIDTH, MENU_HEIGHT);
                    menu.current_state = menu_state_main;
                } else {
                    printf("ERROR: Failed to connect to %s:%d\n", ip, port);
                    menu_set_connection_failed(&menu, "Failed to connect to host");
                    active_network = nullptr;
                }
            }
        }

        // Check if user wants to quit
        else if (menu_should_quit(&menu)) {
            break;
        }

        begin_drawing();
        render_menu(&menu);
        end_drawing();
    }

    if (active_network) {
        network_close(active_network);
    }
    cleanup_menu(&menu);
    close_window();

    return 0;
}
