#ifndef MULTIPLAYER_UI_H
#define MULTIPLAYER_UI_H

#include "raylib.h"

// Forward declarations
// ReSharper disable once CppRedundantElaboratedTypeSpecifier
typedef struct game game;
// ReSharper disable once CppRedundantElaboratedTypeSpecifier
typedef struct network_state network_state;

// Enemy sending button
typedef struct {
    vector2 position;
    vector2 size;
    const char* label;
    int enemy_count;
    int cost;
    bool hovered;
    bool clicked;
} send_enemy_button;

// Multiplayer UI state
typedef struct {
    // Enemy sending buttons
    send_enemy_button send_buttons[3];  // Send 1, 5, 10 enemies

    // Connection status
    bool is_connected;
    bool is_host;

    // UI positioning
    int split_x;  // X position where split happens (800 for 1600px window)
    int game_width;  // Width of each game view (800)
    int game_height; // Height of game area (600)
} multiplayer_ui;

// Initialize multiplayer UI
multiplayer_ui init_multiplayer_ui(bool is_host, int window_width, int window_height);

// Update UI (check button hovers/clicks)
void update_multiplayer_ui(multiplayer_ui* ui);

// Update UI dimensions when window is resized
void update_multiplayer_ui_dimensions(multiplayer_ui* ui, int window_width, int window_height);

// Render split-screen view
void render_split_screen(const multiplayer_ui* ui, const game* local_game, const game* remote_game);

// Render enemy sending UI
void render_enemy_send_ui(const multiplayer_ui* ui, const game* local_game);

// Check if any send button was clicked and return enemy count (0 if none)
int check_send_button_clicked(const multiplayer_ui* ui, int* out_cost);

// Render connection status
void render_connection_status(const multiplayer_ui* ui);

#endif
