#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static Uint64 last_time = 0;
static float delta_time = 0.0f;
static bool* keys_pressed = nullptr;
static bool* mouse_pressed = nullptr;
static int screen_width = 0;
static int screen_height = 0;
static int target_fps = 60;
static TTF_Font* default_font = nullptr;
static unsigned int rprand_state = 0;

void InitWindow(const int width, const int height, const char* const title) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "ERROR: SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }

    constexpr int img_flags = IMG_INIT_PNG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        fprintf(stderr, "ERROR: IMG_Init failed: %s\n", IMG_GetError());
        SDL_Quit();
        exit(1);
    }

    if (TTF_Init() < 0) {
        fprintf(stderr, "ERROR: TTF_Init failed: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "ERROR: SDL_CreateWindow failed: %s\n", SDL_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "ERROR: SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        exit(1);
    }

    keys_pressed = (bool*)calloc(SDL_NUM_SCANCODES, sizeof(bool));
    mouse_pressed = (bool*)calloc(SDL_BUTTON_X2 + 1, sizeof(bool));

    last_time = SDL_GetPerformanceCounter();
    screen_width = width;
    screen_height = height;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void SetTargetFPS(const int fps) {
    target_fps = fps;
}

void CloseWindow(void) {
    if (default_font) TTF_CloseFont(default_font);
    free(keys_pressed);
    free(mouse_pressed);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool WindowShouldClose(void) {
    SDL_Event event;
    memset(keys_pressed, 0, SDL_NUM_SCANCODES * sizeof(bool));
    memset(mouse_pressed, 0, (SDL_BUTTON_X2 + 1) * sizeof(bool));

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return true;
        }
        if (event.type == SDL_KEYDOWN) {
            keys_pressed[event.key.keysym.scancode] = true;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            mouse_pressed[event.button.button] = true;
        }
    }

    const Uint64 current_time = SDL_GetPerformanceCounter();
    delta_time = (float)(current_time - last_time) / (float)SDL_GetPerformanceFrequency();
    last_time = current_time;

    return false;
}

void BeginDrawing(void) {
    int window_w = 0, window_h = 0;
    SDL_GetWindowSize(window, &window_w, &window_h);

    if (window_w != screen_width || window_h != screen_height) {
        screen_width = window_w;
        screen_height = window_h;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void EndDrawing(void) {
    SDL_RenderPresent(renderer);
}

void ClearBackground(const Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

int GetScreenWidth(void) {
    int w = 0;
    SDL_GetWindowSize(window, &w, nullptr);
    return w > 0 ? w : screen_width;
}

int GetScreenHeight(void) {
    int h = 0;
    SDL_GetWindowSize(window, nullptr, &h);
    return h > 0 ? h : screen_height;
}

float GetFrameTime(void) {
    return delta_time;
}

Texture2D LoadTexture(const char* const fileName) {
    SDL_Surface* const surface = IMG_Load(fileName);
    if (!surface) {
        fprintf(stderr, "ERROR: Failed to load texture %s: %s\n", fileName, IMG_GetError());
        return (Texture2D){0, 0, 0};
    }

    SDL_Texture* const texture = SDL_CreateTextureFromSurface(renderer, surface);
    const int width = surface->w;
    const int height = surface->h;
    SDL_FreeSurface(surface);

    if (!texture) {
        fprintf(stderr, "ERROR: Failed to create texture %s: %s\n", fileName, SDL_GetError());
        return (Texture2D){0, 0, 0};
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    return (Texture2D){(uintptr_t)texture, width, height};
}

void UnloadTexture(const Texture2D texture) {
    if (texture.id != 0) {
        SDL_DestroyTexture((SDL_Texture*)(uintptr_t)texture.id);
    }
}

void DrawTexturePro(const Texture2D texture, const Rectangle source, const Rectangle dest, const Vector2 origin, const float rotation, const Color tint) {
    if (texture.id == 0) {
        return;
    }

    SDL_Texture* const sdl_texture = (SDL_Texture*)(uintptr_t)texture.id;

    if (SDL_SetTextureColorMod(sdl_texture, tint.r, tint.g, tint.b) < 0) {
        fprintf(stderr, "ERROR: SDL_SetTextureColorMod failed: %s\n", SDL_GetError());
        return;
    }

    if (SDL_SetTextureAlphaMod(sdl_texture, tint.a) < 0) {
        fprintf(stderr, "ERROR: SDL_SetTextureAlphaMod failed: %s\n", SDL_GetError());
        return;
    }

    const SDL_Rect src_rect = {
        (int)source.x,
        (int)source.y,
        (int)source.width,
        (int)source.height
    };

    const SDL_Rect dst_rect = {
        (int)(dest.x - origin.x),
        (int)(dest.y - origin.y),
        (int)dest.width,
        (int)dest.height
    };

    const SDL_Point center = {(int)origin.x, (int)origin.y};

    if (SDL_RenderCopyEx(renderer, sdl_texture, &src_rect, &dst_rect, (double)rotation, &center, SDL_FLIP_NONE) < 0) {
        fprintf(stderr, "ERROR: SDL_RenderCopyEx failed: %s\n", SDL_GetError());
    }
}

void DrawRectangle(const int posX, const int posY, const int width, const int height, const Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    const SDL_Rect rect = {posX, posY, width, height};
    SDL_RenderFillRect(renderer, &rect);
}

void DrawRectangleLines(const int posX, const int posY, const int width, const int height, const Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    const SDL_Rect rect = {posX, posY, width, height};
    SDL_RenderDrawRect(renderer, &rect);
}

void DrawText(const char* const text, const int posX, const int posY, const int fontSize, const Color color) {
    if (!text) return;

    if (!default_font || TTF_FontHeight(default_font) != fontSize) {
        if (default_font) TTF_CloseFont(default_font);
        default_font = TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSans.ttf", fontSize);
        if (!default_font) {
            default_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        }
        if (!default_font) {
            fprintf(stderr, "ERROR: Failed to load font: %s\n", TTF_GetError());
            return;
        }
    }

    const SDL_Color sdl_color = {color.r, color.g, color.b, color.a};
    SDL_Surface* const surface = TTF_RenderText_Blended(default_font, text, sdl_color);
    if (!surface) {
        fprintf(stderr, "ERROR: Failed to render text: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* const texture = SDL_CreateTextureFromSurface(renderer, surface);
    const int width = surface->w;
    const int height = surface->h;
    SDL_FreeSurface(surface);

    if (!texture) {
        fprintf(stderr, "ERROR: Failed to create text texture: %s\n", SDL_GetError());
        return;
    }

    const SDL_Rect dest = {posX, posY, width, height};
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    SDL_DestroyTexture(texture);
}

int MeasureText(const char* const text, const int fontSize) {
    if (!text) return 0;

    if (!default_font || TTF_FontHeight(default_font) != fontSize) {
        if (default_font) TTF_CloseFont(default_font);
        default_font = TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSans.ttf", fontSize);
        if (!default_font) {
            default_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", fontSize);
        }
        if (!default_font) {
            return (int)strlen(text) * (fontSize / 2);
        }
    }

    int width = 0;
    TTF_SizeText(default_font, text, &width, nullptr);
    return width;
}

void DrawFPS(const int posX, const int posY) {
    const int fps = (int)(1.0f / delta_time);
    char fps_text[32];
    snprintf(fps_text, sizeof(fps_text), "FPS: %d", fps);
    DrawText(fps_text, posX, posY, 20, GREEN);
}

bool IsKeyPressed(const int key) {
    const SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
    return keys_pressed[scancode];
}

bool IsMouseButtonPressed(const int button) {
    return mouse_pressed[button];
}

Vector2 GetMousePosition(void) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return (Vector2){(float)x, (float)y};
}

static int GetRandomValueInternal(const int min, const int max) {
    if (min > max) {
        const int tmp = max;
        const int max_tmp = min;
        const int min_tmp = tmp;
        return GetRandomValueInternal(min_tmp, max_tmp);
    }

    if (rprand_state == 0) {
        rprand_state = (unsigned int)SDL_GetPerformanceCounter();
    }

    rprand_state = rprand_state * 1103515245U + 12345U;
    return min + (int)((rprand_state >> 16) % (unsigned int)(max - min + 1));
}

int GetRandomValue(const int min, const int max) {
    return GetRandomValueInternal(min, max);
}
