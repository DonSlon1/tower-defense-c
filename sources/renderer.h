//
// Created by lukas on 10/13/25.
//

#ifndef PROJEKT_RENDERER_H
#define PROJEKT_RENDERER_H

#include <raylib.h>
#include "game_object.h"

typedef struct GAME GAME;

void draw_game_objects(const GAME* game);

void draw_hud(const GAME* game);

void draw_start_screen(const GAME* game);

void draw_game_over_screen(const GAME* game);

void draw_fullscreen_image(Texture2D texture);

void draw_text_with_shadow(const char* text, int x, int y, int size, Color color);

void draw_centered_text_with_shadow(const char* text, int y, int size, Color color);

void draw_tower_info(const GAME_OBJECT* tower, int x, int y);

void draw_tower_spots(const GAME* game);

void draw_wave_info(const GAME* game);
void draw_wave_break_screen(const GAME* game);

#endif //PROJEKT_RENDERER_H
