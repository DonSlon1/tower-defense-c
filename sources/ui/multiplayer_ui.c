#include "multiplayer_ui.h"
#include "game.h"
#include "renderer.h"
#include <stdio.h>

constexpr float button_width = 120;
constexpr float button_height = 40;
constexpr float button_spacing = 10;
constexpr float button_start_x = 20;
constexpr float button_offset_y = 60;

static void position_send_buttons(multiplayer_ui* ui, const int window_height) {
    const float button_y = (float)window_height - button_offset_y;

    ui->send_buttons[0].position = (vector2){button_start_x, button_y};
    ui->send_buttons[1].position = (vector2){button_start_x + button_width + button_spacing, button_y};
    ui->send_buttons[2].position = (vector2){button_start_x + (button_width + button_spacing) * 2, button_y};
}

// Initialize multiplayer UI
multiplayer_ui init_multiplayer_ui(const bool is_host, const int window_width, const int window_height) {
    multiplayer_ui ui = {0};

    ui.is_host = is_host;
    ui.is_connected = true;
    ui.split_x = window_width / 2;
    ui.game_width = window_width / 2;
    ui.game_height = window_height;

    ui.send_buttons[0] = (send_enemy_button){
        .size = {button_width, button_height},
        .label = "Send 1 ($50)",
        .enemy_count = 1,
        .cost = 50,
        .hovered = false,
        .clicked = false
    };

    ui.send_buttons[1] = (send_enemy_button){
        .size = {button_width, button_height},
        .label = "Send 5 ($200)",
        .enemy_count = 5,
        .cost = 200,
        .hovered = false,
        .clicked = false
    };

    ui.send_buttons[2] = (send_enemy_button){
        .size = {button_width, button_height},
        .label = "Send 10 ($350)",
        .enemy_count = 10,
        .cost = 350,
        .hovered = false,
        .clicked = false
    };

    position_send_buttons(&ui, window_height);

    return ui;
}

// Check if mouse is over a button
static bool is_mouse_over_button(const send_enemy_button* button) {
    const vector2 mouse_pos = get_mouse_position();
    return mouse_pos.x >= button->position.x &&
           mouse_pos.x <= button->position.x + button->size.x &&
           mouse_pos.y >= button->position.y &&
           mouse_pos.y <= button->position.y + button->size.y;
}

// Update UI dimensions when window is resized
void update_multiplayer_ui_dimensions(multiplayer_ui* ui, const int window_width, const int window_height) {
    if (!ui) return;

    ui->split_x = window_width / 2;
    ui->game_width = window_width / 2;
    ui->game_height = window_height;

    position_send_buttons(ui, window_height);
}

// Update UI (check button hovers/clicks)
void update_multiplayer_ui(multiplayer_ui* ui) {
    if (!ui) return;

    const bool mouse_clicked = is_mouse_button_pressed(SDL_BUTTON_LEFT);

    // Update send buttons
    for (int i = 0; i < 3; i++) {
        ui->send_buttons[i].hovered = is_mouse_over_button(&ui->send_buttons[i]);
        ui->send_buttons[i].clicked = ui->send_buttons[i].hovered && mouse_clicked;
    }
}

// Render split-screen UI elements (divider and labels)
// Note: Actual game rendering is done in main.c using SDL viewports
void render_split_screen(const multiplayer_ui* ui, [[maybe_unused]] const game* local_game, [[maybe_unused]] const game* remote_game) {
    if (!ui) return;

    // Draw dividing line (vertical line using thin rectangle)
    draw_rectangle(ui->split_x - 1, 0, 2, ui->game_height, white);

    // Draw labels
    draw_text("YOUR GAME", 20, 10, 20, green);
    draw_text("OPPONENT", ui->split_x + 20, 10, 20, red);
}

// Render a single send button
static void render_send_button(const send_enemy_button* button, const bool can_afford) {
    // Choose color based on state
    color button_color;
    if (!can_afford) {
        button_color = (color){60, 60, 60, 255};  // Gray if user can't afford
    } else if (button->hovered) {
        button_color = (color){100, 150, 100, 255};  // Light green on hover
    } else {
        button_color = (color){50, 100, 50, 255};   // Dark green
    }

    // Draw button background
    draw_rectangle(
        (int)button->position.x,
        (int)button->position.y,
        (int)button->size.x,
        (int)button->size.y,
        button_color
    );

    // Draw border
    const color border_color = button->hovered ? green : white;
    draw_rectangle_lines(
       (int)button->position.x,
        (int)button->position.y,
        (int)button->size.x,
        (int)button->size.y,
        border_color
    );

    // Draw text
    const color text_color = can_afford ? white : (color){120, 120, 120, 255};
    const int text_x = (int)button->position.x + 10;
    const int text_y = (int)button->position.y + 12;
    draw_text(button->label, text_x, text_y, 16, text_color);
}

// Render enemy sending UI
void render_enemy_send_ui(const multiplayer_ui* ui, const game* local_game) {
    if (!ui || !local_game) return;

    // Draw background panel
    const auto panel_color = (color){30, 30, 40, 200};
    draw_rectangle(0, ui->game_height - 70, ui->game_width, 70, panel_color);

    // Draw label
    draw_text("SEND ENEMIES TO OPPONENT:", 20, ui->game_height - 80, 16, gold);

    // Draw buttons
    for (int i = 0; i < 3; i++) {
        const bool can_afford = local_game->player_money >= ui->send_buttons[i].cost;
        render_send_button(&ui->send_buttons[i], can_afford);
    }
}

// Check if any send button was clicked and return enemy count (0 if none)
int check_send_button_clicked(const multiplayer_ui* ui, int* out_cost) {
    if (!ui || !out_cost) return 0;

    for (int i = 0; i < 3; i++) {
        if (ui->send_buttons[i].clicked) {
            *out_cost = ui->send_buttons[i].cost;
            return ui->send_buttons[i].enemy_count;
        }
    }

    return 0;  // No button clicked
}

// Render connection status
void render_connection_status(const multiplayer_ui* ui) {
    if (!ui) return;

    const char* status_text = ui->is_connected ? "CONNECTED" : "DISCONNECTED";
    const color status_color = ui->is_connected ? green : red;

    draw_text(status_text, ui->split_x - 80, 10, 16, status_color);
}
