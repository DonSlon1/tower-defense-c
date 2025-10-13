//
// Created by lukas on 10/13/25.
//

#ifndef PROJEKT_RENDERER_H
#define PROJEKT_RENDERER_H

#include <raylib.h>

// Forward declaration
typedef struct GAME GAME;

// Draw all game objects (towers, enemies, etc.)
void draw_game_objects(const GAME* game);

// Draw the heads-up display (lives, money, score)
void draw_hud(const GAME* game);

// Draw the start screen
void draw_start_screen(const GAME* game);

// Draw the game over screen
void draw_game_over_screen(const GAME* game);

// Helper function to draw a fullscreen image
void draw_fullscreen_image(Texture2D texture);

// Helper function to draw text with shadow
void draw_text_with_shadow(const char* text, int x, int y, int size, Color color);

// Helper function to draw centered text with shadow
void draw_centered_text_with_shadow(const char* text, int y, int size, Color color);

#endif //PROJEKT_RENDERER_H
