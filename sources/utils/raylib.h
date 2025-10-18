#ifndef RAYLIB_H
#define RAYLIB_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>

typedef struct vector2 {
    float x;
    float y;
} vector2;

typedef struct rectangle {
    float x;
    float y;
    float width;
    float height;
} rectangle;

typedef struct color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} color;

typedef struct texture_2d {
    uintptr_t id;
    int width;
    int height;
} texture_2d;

constexpr color white = {255, 255, 255, 255};
constexpr color black = {0, 0, 0, 255};
constexpr color red = {255, 0, 0, 255};
constexpr color green = {0, 255, 0, 255};
constexpr color skyblue = {135, 206, 235, 255};
constexpr color gold = {255, 215, 0, 255};
constexpr color lightgray = {200, 200, 200, 255};

constexpr int key_space = SDLK_SPACE;
constexpr int mouse_button_left = SDL_BUTTON_LEFT;
constexpr int mouse_button_right = SDL_BUTTON_RIGHT;

void init_window(int width, int height, const char* title);
void set_window_size(int width, int height);
void set_target_fps(int fps);
void close_window(void);
bool window_should_close(void);
void set_viewport(int x, int y, int width, int height);
void reset_viewport(void);
void begin_drawing(void);
void end_drawing(void);
void clear_background(color color);
int get_screen_width(void);
int get_screen_height(void);
float get_frame_time(void);

texture_2d load_texture(const char* file_name);
void unload_texture(texture_2d texture);
void draw_texture_pro(texture_2d texture, rectangle source, rectangle dest, vector2 origin, float rotation, color tint);
void draw_rectangle(int pos_x, int pos_y, int width, int height, color color);
void draw_rectangle_lines(int pos_x, int pos_y, int width, int height, color color);
void draw_text(const char* text, int pos_x, int pos_y, int font_size, color color);
int measure_text(const char* text, int font_size);
void draw_fps(int pos_x, int pos_y);

bool is_key_pressed(int key);
bool is_mouse_button_pressed(int button);
vector2 get_mouse_position(void);
int get_random_value(int min, int max);

// Text input
int get_char_pressed(void);  // Returns ASCII char pressed this frame, or 0 if none

void set_mouse_cursor(const char* file_name);
void set_mouse_pointer(const char* file_name);
void use_normal_cursor(void);
void use_pointer_cursor(void);
void show_cursor(void);
void hide_cursor(void);

void set_window_icon(const char* file_name);

#endif
