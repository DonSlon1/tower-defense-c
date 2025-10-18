#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "../network/network.h"

// Menu states - tracks which screen we're on
typedef enum {
    menu_state_main,                    // Main menu: Single/Multi/Quit
    menu_state_multiplayer_select,      // Multiplayer: Host/Join/Back
    menu_state_host_setup,              // Configure host settings (port, name)
    menu_state_host_waiting,            // Waiting for player to connect
    menu_state_join_selecting_game,     // Browse discovered games (with discovery)
    menu_state_join_entering_ip,        // Manual IP entry
    menu_state_connecting,              // Attempting to connect
    menu_state_playing_single,          // Single player game active
    menu_state_playing_multiplayer,     // Multiplayer game active
    menu_state_quit                     // Quit application
} menu_state;

// Button structure for UI elements
typedef struct {
    vector2 position;
    vector2 size;
    const char* text;
    bool hovered;
    bool clicked;
} menu_button;

// Discovered game info (for Phase 2.5 - game discovery)
#define MAX_DISCOVERED_GAMES 16

typedef struct {
    char host_ip[64];          // IP address to connect to
    uint16_t game_port;        // TCP port (7777)
    char host_name[32];        // Host player name
    uint32_t last_seen_ms;     // Timestamp
    bool is_active;            // Still broadcasting
} discovered_game;

// Main menu system structure
typedef struct {
    menu_state current_state;
    menu_state previous_state;

    // IP/Port input
    char ip_input[64];
    int ip_cursor_pos;
    bool ip_input_active;

    char port_input[8];
    int port_cursor_pos;
    bool port_input_active;
    int port_number;

    // Player name input (for later)
    char player_name[32];

    // Buttons for each menu state
    menu_button main_buttons[3];        // Single Player, Multiplayer, Quit
    menu_button multi_buttons[3];       // Host, Join, Back
    menu_button ip_buttons[2];          // Connect, Cancel
    menu_button host_setup_buttons[2];  // Start Hosting, Cancel
    menu_button host_buttons[1];        // Cancel
    menu_button game_buttons[MAX_DISCOVERED_GAMES];  // One per discovered game
    menu_button manual_ip_button;       // "Enter IP Manually" fallback
    menu_button refresh_button;         // Refresh game list
    menu_button back_button;            // Back from game browser

    // Network/connection status
    bool is_host;
    bool waiting_for_player;
    bool connection_failed;
    char error_message[128];

    // Game discovery
    session_discovery* discovery;
    discovered_session available_games[MAX_DISCOVERED_GAMES];
    int available_game_count;
    int selected_game_index;

    // Host name for broadcasting
    char host_name_input[64];
    int host_name_cursor_pos;
    bool host_name_input_active;

} menu_system;

// Main menu API
menu_system init_menu_system(void);
void update_menu(menu_system* menu);
void render_menu(const menu_system* menu);
void cleanup_menu(menu_system* menu);

// State query functions - use these in main.c to check what to do
bool menu_should_start_singleplayer(const menu_system* menu);
bool menu_should_start_multiplayer(const menu_system* menu);
bool menu_should_quit(const menu_system* menu);

// Get connection info for networking (Phase 2)
const char* menu_get_ip(const menu_system* menu);
int menu_get_port(const menu_system* menu);
bool menu_is_host(const menu_system* menu);
const char* menu_get_player_name(const menu_system* menu);

// Network status updates - call these from your network code
void menu_set_connection_failed(menu_system* menu, const char* error);
void menu_set_connection_success(menu_system* menu);
void menu_set_player_connected(menu_system* menu);

#endif
