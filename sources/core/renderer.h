//
// Created by lukas on 10/13/25.
//

#ifndef PROJEKT_RENDERER_H
#define PROJEKT_RENDERER_H

#include <raylib.h>
#include "game_object.h"

typedef struct game game;

void draw_game_objects(const game* g);

void draw_hud(const game* g);

void draw_start_screen(const game* g);

void draw_game_over_screen(const game* g);

void draw_fullscreen_image(texture_2d texture);

void draw_text_with_shadow(const char* text, int x, int y, int size, color c);

void draw_centered_text_with_shadow(const char* text, int y, int size, color c);

void draw_tower_info(const game_object* tower_object, int x, int y);

void draw_tower_spots(const game* g);

void draw_wave_info(const game* g);
void draw_wave_break_screen(const game* g);

texture_2d get_enemy_texture(const game* g, enemy_type type, enemy_animation_state state);

#endif //PROJEKT_RENDERER_H
