#include "menu.h"
#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Helper: Update button state (hover, click detection)
static void update_button(menu_button* btn) {
    if (!btn) return;

    const vector2 mouse = get_mouse_position();

    btn->hovered = (mouse.x >= btn->position.x &&
                   mouse.x <= btn->position.x + btn->size.x &&
                   mouse.y >= btn->position.y &&
                   mouse.y <= btn->position.y + btn->size.y);

    btn->clicked = btn->hovered && is_mouse_button_pressed(mouse_button_left);
}

// Helper: Render button with hover effect
static void render_button(const menu_button* btn) {
    if (!btn || !btn->text) return;

    const color bg_color = btn->hovered ?
        (color){100, 100, 100, 255} :
        (color){60, 60, 60, 255};
    const color border_color = btn->hovered ?
        white :
        (color){150, 150, 150, 255};

    // Draw background
    draw_rectangle(
        (int)btn->position.x,
        (int)btn->position.y,
        (int)btn->size.x,
        (int)btn->size.y,
        bg_color
    );

    // Draw border
    draw_rectangle_lines(
        (int)btn->position.x,
        (int)btn->position.y,
        (int)btn->size.x,
        (int)btn->size.y,
        border_color
    );

    // Draw text centered
    const int text_width = measure_text(btn->text, 20);
    const int text_x = (int)btn->position.x + ((int)btn->size.x - text_width) / 2;
    const int text_y = (int)btn->position.y + ((int)btn->size.y - 20) / 2;

    draw_text(btn->text, text_x, text_y, 20, white);
}

// Initialize menu system
menu_system init_menu_system(void) {
    menu_system menu = {0};
    menu.current_state = MENU_STATE_MAIN;

    // Default values
    strcpy(menu.ip_input, "127.0.0.1");
    strcpy(menu.port_input, "7777");
    strcpy(menu.player_name, "Player");
    strcpy(menu.host_name_input, "My Game");
    menu.port_number = 7777;
    menu.ip_cursor_pos = (int)strlen(menu.ip_input);
    menu.port_cursor_pos = (int)strlen(menu.port_input);
    menu.host_name_cursor_pos = (int)strlen(menu.host_name_input);

    // Initialize main menu buttons (centered on 800x600)
    menu.main_buttons[0] = (menu_button){
        .position = {300, 200},
        .size = {200, 50},
        .text = "Single Player"
    };
    menu.main_buttons[1] = (menu_button){
        .position = {300, 270},
        .size = {200, 50},
        .text = "Multiplayer"
    };
    menu.main_buttons[2] = (menu_button){
        .position = {300, 340},
        .size = {200, 50},
        .text = "Quit"
    };

    // Initialize multiplayer menu buttons
    menu.multi_buttons[0] = (menu_button){
        .position = {300, 200},
        .size = {200, 50},
        .text = "Host Game"
    };
    menu.multi_buttons[1] = (menu_button){
        .position = {300, 270},
        .size = {200, 50},
        .text = "Join Game"
    };
    menu.multi_buttons[2] = (menu_button){
        .position = {300, 340},
        .size = {200, 50},
        .text = "Back"
    };

    // IP entry screen buttons
    menu.ip_buttons[0] = (menu_button){
        .position = {250, 350},
        .size = {120, 40},
        .text = "Connect"
    };
    menu.ip_buttons[1] = (menu_button){
        .position = {390, 350},
        .size = {120, 40},
        .text = "Cancel"
    };

    // Host setup screen buttons
    menu.host_setup_buttons[0] = (menu_button){
        .position = {250, 380},
        .size = {150, 40},
        .text = "Start Hosting"
    };
    menu.host_setup_buttons[1] = (menu_button){
        .position = {420, 380},
        .size = {120, 40},
        .text = "Cancel"
    };

    // Host waiting screen button
    menu.host_buttons[0] = (menu_button){
        .position = {340, 400},
        .size = {120, 40},
        .text = "Cancel"
    };

    // Game browser buttons
    menu.refresh_button = (menu_button){
        .position = {50, 480},
        .size = {120, 40},
        .text = "Refresh"
    };
    menu.manual_ip_button = (menu_button){
        .position = {200, 480},
        .size = {200, 40},
        .text = "Enter IP Manually"
    };
    menu.back_button = (menu_button){
        .position = {420, 480},
        .size = {160, 40},
        .text = "Back"
    };

    return menu;
}

// Update main menu
static void update_main_menu(menu_system* menu) {
    for (int i = 0; i < 3; i++) {
        update_button(&menu->main_buttons[i]);
    }

    if (menu->main_buttons[0].clicked) {
        // Single Player
        menu->current_state = MENU_STATE_PLAYING_SINGLE;
    } else if (menu->main_buttons[1].clicked) {
        // Multiplayer
        menu->current_state = MENU_STATE_MULTIPLAYER_SELECT;
    } else if (menu->main_buttons[2].clicked) {
        // Quit
        menu->current_state = MENU_STATE_QUIT;
    }
}

// Update multiplayer selection menu
static void update_multiplayer_menu(menu_system* menu) {
    for (int i = 0; i < 3; i++) {
        update_button(&menu->multi_buttons[i]);
    }

    if (menu->multi_buttons[0].clicked) {
        // Host Game - go to setup screen
        menu->is_host = true;
        menu->current_state = MENU_STATE_HOST_SETUP;
        menu->port_input_active = false;
        menu->host_name_input_active = false;

    } else if (menu->multi_buttons[1].clicked) {
        // Join Game - start discovery and show game browser
        menu->is_host = false;

        // Create discovery service (use port 0 for auto-assign, client doesn't need specific port)
        menu->discovery = discovery_create(0);
        if (menu->discovery) {
            menu->available_game_count = discovery_find_sessions(
                menu->discovery,
                menu->available_games,
                MAX_DISCOVERED_GAMES,
                2.0f  // 2 second timeout
            );
        } else {
            // Discovery failed, but we can still use manual IP entry
            menu->available_game_count = 0;
        }

        menu->current_state = MENU_STATE_JOIN_SELECTING_GAME;

    } else if (menu->multi_buttons[2].clicked) {
        // Back
        menu->current_state = MENU_STATE_MAIN;
    }
}

// Update IP input field
static void update_ip_input(menu_system* menu) {
    if (!menu->ip_input_active) return;

    // TODO (Phase 1.5): Implement proper text input with SDL_TextInputEvent
    // For now, IP input is pre-filled with 127.0.0.1
    // You can manually edit it in the code if needed

    // Handle backspace
    if (is_key_pressed(SDLK_BACKSPACE) && menu->ip_cursor_pos > 0) {
        menu->ip_cursor_pos--;
        menu->ip_input[menu->ip_cursor_pos] = '\0';
    }
}

// Update join/IP entry menu
static void update_join_ip_menu(menu_system* menu) {
    update_ip_input(menu);

    for (int i = 0; i < 2; i++) {
        update_button(&menu->ip_buttons[i]);
    }

    if (menu->ip_buttons[0].clicked) {
        // Connect
        menu->current_state = MENU_STATE_CONNECTING;
        menu->ip_input_active = false;
        // Network connection will be attempted in main.c

    } else if (menu->ip_buttons[1].clicked) {
        // Cancel
        menu->current_state = MENU_STATE_MULTIPLAYER_SELECT;
        menu->ip_input_active = false;
    }
}

// Update host setup screen
static void update_host_setup(menu_system* menu) {
    // Handle port input
    if (menu->port_input_active && is_key_pressed(SDLK_BACKSPACE) && menu->port_cursor_pos > 0) {
        menu->port_cursor_pos--;
        menu->port_input[menu->port_cursor_pos] = '\0';
        menu->port_number = atoi(menu->port_input);
    }

    // Update buttons
    for (int i = 0; i < 2; i++) {
        update_button(&menu->host_setup_buttons[i]);
    }

    if (menu->host_setup_buttons[0].clicked) {
        // Start Hosting
        menu->port_number = atoi(menu->port_input);
        if (menu->port_number <= 0 || menu->port_number > 65535) {
            menu->port_number = 7777;
            strcpy(menu->port_input, "7777");
        }

        menu->waiting_for_player = true;
        menu->current_state = MENU_STATE_HOST_WAITING;

        // Start discovery broadcaster
        menu->discovery = discovery_create(47777);
        if (menu->discovery) {
            discovery_start_host(menu->discovery, menu->host_name_input, menu->port_number);
        }

    } else if (menu->host_setup_buttons[1].clicked) {
        // Cancel
        menu->current_state = MENU_STATE_MULTIPLAYER_SELECT;
        menu->port_input_active = false;
        menu->host_name_input_active = false;
    }
}

// Update host waiting screen
static void update_host_waiting(menu_system* menu) {
    update_button(&menu->host_buttons[0]);

    // Respond to discovery requests
    if (menu->discovery) {
        discovery_host_update(menu->discovery);
    }

    if (menu->host_buttons[0].clicked) {
        // Cancel hosting
        menu->current_state = MENU_STATE_MULTIPLAYER_SELECT;
        menu->waiting_for_player = false;

        // Stop discovery
        if (menu->discovery) {
            discovery_close(menu->discovery);
            menu->discovery = nullptr;
        }
    }
}

// Update game browser
static void update_join_browser(menu_system* menu) {
    // Initialize game buttons based on discovered sessions
    for (int i = 0; i < menu->available_game_count && i < 8; i++) {
        menu->game_buttons[i].position = (vector2){100, 170 + i * 35};
        menu->game_buttons[i].size = (vector2){600, 30};
        menu->game_buttons[i].text = "";  // We'll draw custom text in render
    }

    // Update buttons
    update_button(&menu->refresh_button);
    update_button(&menu->manual_ip_button);
    update_button(&menu->back_button);

    // Handle refresh button
    if (menu->refresh_button.clicked) {
        // Close existing discovery if any
        if (menu->discovery) {
            discovery_close(menu->discovery);
            menu->discovery = nullptr;
        }

        // Create new discovery and scan again
        menu->discovery = discovery_create(0);
        if (menu->discovery) {
            menu->available_game_count = discovery_find_sessions(
                menu->discovery,
                menu->available_games,
                MAX_DISCOVERED_GAMES,
                2.0f  // 2 second timeout
            );
        } else {
            menu->available_game_count = 0;
        }
    }

    // Update discovered game buttons
    for (int i = 0; i < menu->available_game_count && i < 8; i++) {
        update_button(&menu->game_buttons[i]);
        if (menu->game_buttons[i].clicked) {
            // Connect to selected game
            strncpy(menu->ip_input, menu->available_games[i].ip_address, sizeof(menu->ip_input) - 1);
            menu->ip_input[sizeof(menu->ip_input) - 1] = '\0';
            menu->port_number = menu->available_games[i].port;
            menu->current_state = MENU_STATE_CONNECTING;

            // Clean up discovery
            if (menu->discovery) {
                discovery_close(menu->discovery);
                menu->discovery = nullptr;
            }
        }
    }

    if (menu->manual_ip_button.clicked) {
        menu->current_state = MENU_STATE_JOIN_ENTERING_IP;
        menu->ip_input_active = true;

        // Clean up discovery
        if (menu->discovery) {
            discovery_close(menu->discovery);
            menu->discovery = nullptr;
        }
    }

    if (menu->back_button.clicked) {
        menu->current_state = MENU_STATE_MULTIPLAYER_SELECT;

        // Clean up discovery
        if (menu->discovery) {
            discovery_close(menu->discovery);
            menu->discovery = nullptr;
        }
    }
}

// Update connecting screen
static void update_connecting(menu_system* menu) {
    // Connection is handled in main.c
    // This screen just shows "Connecting..."
    // main.c will call menu_set_connection_success() or menu_set_connection_failed()

    // Allow ESC to cancel
    if (is_key_pressed(SDLK_ESCAPE) || menu->connection_failed) {
        menu->current_state = MENU_STATE_MULTIPLAYER_SELECT;
        menu->connection_failed = false;
    }
}

// Main update function - routes to correct state
void update_menu(menu_system* menu) {
    if (!menu) return;

    switch (menu->current_state) {
        case MENU_STATE_MAIN:
            update_main_menu(menu);
            break;

        case MENU_STATE_MULTIPLAYER_SELECT:
            update_multiplayer_menu(menu);
            break;

        case MENU_STATE_HOST_SETUP:
            update_host_setup(menu);
            break;

        case MENU_STATE_HOST_WAITING:
            update_host_waiting(menu);
            break;

        case MENU_STATE_JOIN_SELECTING_GAME:
            update_join_browser(menu);
            break;

        case MENU_STATE_JOIN_ENTERING_IP:
            update_join_ip_menu(menu);
            break;

        case MENU_STATE_CONNECTING:
            update_connecting(menu);
            break;

        default:
            break;
    }
}

// Render main menu
static void render_main_menu(const menu_system* menu) {
    draw_text("TOWER DEFENSE", 250, 100, 40, white);

    for (int i = 0; i < 3; i++) {
        render_button(&menu->main_buttons[i]);
    }

    draw_text("Use mouse to select", 300, 450, 16, lightgray);
}

// Render multiplayer selection menu
static void render_multiplayer_menu(const menu_system* menu) {
    draw_text("MULTIPLAYER", 280, 100, 40, white);

    for (int i = 0; i < 3; i++) {
        render_button(&menu->multi_buttons[i]);
    }
}

// Render IP entry screen
static void render_join_ip_menu(const menu_system* menu) {
    draw_text("JOIN GAME", 300, 100, 40, white);

    // Draw IP input label
    draw_text("Enter Host IP Address:", 250, 200, 20, white);

    // Draw IP input box
    const color input_bg = menu->ip_input_active ?
        (color){80, 80, 80, 255} :
        (color){60, 60, 60, 255};
    draw_rectangle(250, 230, 300, 40, input_bg);
    draw_rectangle_lines(250, 230, 300, 40, white);

    // Draw IP text
    draw_text(menu->ip_input, 260, 240, 20, white);

    // Draw blinking cursor
    if (menu->ip_input_active && ((SDL_GetTicks() / 500) % 2 == 0)) {
        const int cursor_x = 260 + measure_text(menu->ip_input, 20);
        draw_rectangle(cursor_x, 240, 2, 20, white);
    }

    // Draw port info
    char port_text[64];
    snprintf(port_text, sizeof(port_text), "Port: %d", menu->port_number);
    draw_text(port_text, 250, 290, 20, lightgray);

    // Draw buttons
    for (int i = 0; i < 2; i++) {
        render_button(&menu->ip_buttons[i]);
    }

    // Draw help text
    draw_text("For local testing use: 127.0.0.1", 200, 420, 16, lightgray);
    draw_text("For LAN use host's local IP: 192.168.x.x", 180, 445, 16, lightgray);
}

// Render host setup screen
static void render_host_setup(const menu_system* menu) {
    draw_text("HOST GAME SETUP", 250, 100, 36, white);

    // Host name input
    draw_text("Session Name:", 250, 180, 20, white);
    const color name_input_bg = menu->host_name_input_active ?
        (color){80, 80, 80, 255} : (color){60, 60, 60, 255};
    draw_rectangle(250, 210, 300, 40, name_input_bg);
    draw_rectangle_lines(250, 210, 300, 40, white);
    draw_text(menu->host_name_input, 260, 220, 20, white);

    // Port input
    draw_text("Port:", 250, 270, 20, white);
    const color port_input_bg = menu->port_input_active ?
        (color){80, 80, 80, 255} : (color){60, 60, 60, 255};
    draw_rectangle(250, 300, 300, 40, port_input_bg);
    draw_rectangle_lines(250, 300, 300, 40, white);
    draw_text(menu->port_input, 260, 310, 20, white);

    draw_text("Press port field to edit", 260, 345, 14, lightgray);

    // Buttons
    for (int i = 0; i < 2; i++) {
        render_button(&menu->host_setup_buttons[i]);
    }
}

// Render host waiting screen
static void render_host_waiting(const menu_system* menu) {
    draw_text("HOSTING GAME", 280, 100, 40, white);

    draw_text("Waiting for player to connect...", 200, 200, 24, white);

    // Animated dots
    const int dot_count = (SDL_GetTicks() / 500) % 4;
    char dots[8] = "";
    for (int i = 0; i < dot_count; i++) {
        strcat(dots, ".");
    }
    draw_text(dots, 560, 200, 24, white);

    // Instructions
    draw_text("Tell the other player to connect to:", 200, 260, 18, lightgray);
    draw_text("127.0.0.1 (if testing on same PC)", 220, 290, 20, gold);
    draw_text("OR your local IP (use 'ip addr' command)", 180, 320, 20, gold);

    char port_text[64];
    snprintf(port_text, sizeof(port_text), "Port: %d", menu->port_number);
    draw_text(port_text, 350, 350, 20, white);

    // Cancel button
    render_button(&menu->host_buttons[0]);
}

// Render game browser
static void render_join_browser(const menu_system* menu) {
    draw_text("AVAILABLE GAMES", 240, 80, 30, white);

    if (menu->available_game_count == 0) {
        draw_text("No games found on local network", 220, 250, 18, lightgray);
        draw_text("Make sure the host is running", 230, 280, 16, lightgray);
    } else {
        // Render discovered games
        draw_text("Select a game to join:", 250, 130, 18, white);

        for (int i = 0; i < menu->available_game_count && i < 8; i++) {
            const int y = 170 + i * 35;

            // Draw game info text
            char game_info[128];
            snprintf(game_info, sizeof(game_info), "%s - %s:%d",
                    menu->available_games[i].host_name,
                    menu->available_games[i].ip_address,
                    menu->available_games[i].port);

            const bool hovered = menu->game_buttons[i].hovered;
            const color bg_color = hovered ? (color){80, 120, 80, 255} : (color){50, 70, 50, 255};
            const color text_color = hovered ? white : lightgray;

            // Draw background box
            draw_rectangle(100, y, 600, 30, bg_color);
            draw_rectangle_lines(100, y, 600, 30, hovered ? green : (color){100, 100, 100, 255});

            // Draw text
            draw_text(game_info, 110, y + 6, 16, text_color);
        }
    }

    // Bottom buttons
    render_button(&menu->refresh_button);
    draw_text("Or:", 370, 450, 16, lightgray);
    render_button(&menu->manual_ip_button);
    render_button(&menu->back_button);
}

// Render connecting screen
static void render_connecting(const menu_system* menu) {
    draw_text("CONNECTING", 300, 250, 30, white);

    char msg[128];
    snprintf(msg, sizeof(msg), "Connecting to %s:%d...",
             menu->ip_input, menu->port_number);
    draw_text(msg, 200, 300, 20, white);

    if (menu->connection_failed) {
        draw_text("Connection Failed!", 270, 350, 24, red);
        draw_text(menu->error_message, 200, 380, 16, red);
        draw_text("Press ESC to go back", 260, 420, 18, lightgray);
    }
}

// Main render function - routes to correct state
void render_menu(const menu_system* menu) {
    if (!menu) return;

    clear_background((color){20, 20, 30, 255});

    switch (menu->current_state) {
        case MENU_STATE_MAIN:
            render_main_menu(menu);
            break;

        case MENU_STATE_MULTIPLAYER_SELECT:
            render_multiplayer_menu(menu);
            break;

        case MENU_STATE_HOST_SETUP:
            render_host_setup(menu);
            break;

        case MENU_STATE_HOST_WAITING:
            render_host_waiting(menu);
            break;

        case MENU_STATE_JOIN_SELECTING_GAME:
            render_join_browser(menu);
            break;

        case MENU_STATE_JOIN_ENTERING_IP:
            render_join_ip_menu(menu);
            break;

        case MENU_STATE_CONNECTING:
            render_connecting(menu);
            break;

        default:
            break;
    }
}

// Cleanup
void cleanup_menu(menu_system* menu) {
    if (!menu) return;

    // Clean up discovery
    if (menu->discovery) {
        discovery_close(menu->discovery);
        menu->discovery = nullptr;
    }
}

// Query functions for main.c
bool menu_should_start_singleplayer(const menu_system* menu) {
    return menu && menu->current_state == MENU_STATE_PLAYING_SINGLE;
}

bool menu_should_start_multiplayer(const menu_system* menu) {
    return menu && (menu->current_state == MENU_STATE_PLAYING_MULTIPLAYER ||
                    menu->current_state == MENU_STATE_CONNECTING ||
                    menu->current_state == MENU_STATE_HOST_WAITING);
}

bool menu_should_quit(const menu_system* menu) {
    return menu && menu->current_state == MENU_STATE_QUIT;
}

const char* menu_get_ip(const menu_system* menu) {
    return menu ? menu->ip_input : "127.0.0.1";
}

int menu_get_port(const menu_system* menu) {
    return menu ? menu->port_number : 7777;
}

bool menu_is_host(const menu_system* menu) {
    return menu && menu->is_host;
}

const char* menu_get_player_name(const menu_system* menu) {
    return menu ? menu->player_name : "Player";
}

// Network status callbacks - you'll call these from your network code
void menu_set_connection_failed(menu_system* menu, const char* error) {
    if (!menu) return;

    menu->connection_failed = true;
    if (error) {
        strncpy(menu->error_message, error, 127);
        menu->error_message[127] = '\0';
    } else {
        strcpy(menu->error_message, "Unknown error");
    }
}

void menu_set_connection_success(menu_system* menu) {
    if (!menu) return;

    menu->connection_failed = false;
    menu->current_state = MENU_STATE_PLAYING_MULTIPLAYER;
}

void menu_set_player_connected(menu_system* menu) {
    if (!menu) return;

    menu->waiting_for_player = false;
    menu->current_state = MENU_STATE_PLAYING_MULTIPLAYER;

    // TODO (Phase 2.5): Stop broadcasting
    // if (menu->discovery) {
    //     discovery_close(menu->discovery);
    //     menu->discovery = NULL;
    // }
}
