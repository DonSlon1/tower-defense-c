#ifndef RAYLIB_H
#define RAYLIB_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>

typedef struct Vector2 {
    float x;
    float y;
} Vector2;

typedef struct Rectangle {
    float x;
    float y;
    float width;
    float height;
} Rectangle;

typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

typedef struct Texture2D {
    uintptr_t id;
    int width;
    int height;
} Texture2D;

constexpr Color WHITE = {255, 255, 255, 255};
constexpr Color BLACK = {0, 0, 0, 255};
constexpr Color RED = {255, 0, 0, 255};
constexpr Color GREEN = {0, 255, 0, 255};
constexpr Color SKYBLUE = {135, 206, 235, 255};
constexpr Color GOLD = {255, 215, 0, 255};
constexpr Color LIGHTGRAY = {200, 200, 200, 255};

constexpr int KEY_SPACE = SDLK_SPACE;
constexpr int MOUSE_BUTTON_LEFT = SDL_BUTTON_LEFT;
constexpr int MOUSE_BUTTON_RIGHT = SDL_BUTTON_RIGHT;

void InitWindow(int width, int height, const char* title);
void SetTargetFPS(int fps);
void CloseWindow(void);
bool WindowShouldClose(void);
void BeginDrawing(void);
void EndDrawing(void);
void ClearBackground(Color color);
int GetScreenWidth(void);
int GetScreenHeight(void);
float GetFrameTime(void);

Texture2D LoadTexture(const char* fileName);
void UnloadTexture(Texture2D texture);
void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);
void DrawRectangle(int posX, int posY, int width, int height, Color color);
void DrawRectangleLines(int posX, int posY, int width, int height, Color color);
void DrawText(const char* text, int posX, int posY, int fontSize, Color color);
int MeasureText(const char* text, int fontSize);
void DrawFPS(int posX, int posY);

bool IsKeyPressed(int key);
bool IsMouseButtonPressed(int button);
Vector2 GetMousePosition(void);
int GetRandomValue(int min, int max);

void SetMouseCursor(const char* fileName);
void set_mouse_pointer(const char* fileName);
void UseNormalCursor(void);
void UsePointerCursor(void);
void ShowCursor(void);
void HideCursor(void);

void SetWindowIcon(const char* fileName);

#endif
