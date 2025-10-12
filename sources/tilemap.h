#ifndef TILEMAP_H
#define TILEMAP_H

#include "raylib.h"

#define TILE_SIZE 16
#define MAP_WIDTH 25
#define MAP_HEIGHT 20

typedef struct {
    int layer1[MAP_HEIGHT][MAP_WIDTH];
    int layer2[MAP_HEIGHT][MAP_WIDTH];
    Texture2D tileset1;  // For layer 1
    Texture2D tileset2;  // For layer 2
    int tile_size;
    int map_width;
    int map_height;
} TILE_MAP;

// Function declarations
TILE_MAP init_tilemap(void);
void draw_tilemap(TILE_MAP* map);
void unload_tilemap(const TILE_MAP* map);
int get_tile_scale(const TILE_MAP* map);
void draw_texture(const TILE_MAP* map, Texture2D tileset, int tile_index, const int x, const int y);
#endif // TILEMAP_H
