#ifndef TILEMAP_H
#define TILEMAP_H

#include "raylib.h"

#define TILE_SIZE 16
#define MAP_WIDTH 25
#define MAP_HEIGHT 20

typedef struct {
    int layer1[MAP_HEIGHT][MAP_WIDTH];
    int layer3[MAP_HEIGHT][MAP_WIDTH];
    int layer4[MAP_HEIGHT][MAP_WIDTH];
    Texture2D tileset1;  // For layer 1
    Texture2D tileset3;  // For layer 3
    Texture2D tileset4;  // For layer 4
    int tile_size;
    int map_width;
    int map_height;
} TILE_MAP;

// Function declarations
TILE_MAP init_tilemap(void);
void draw_tilemap(TILE_MAP* map);
void unload_tilemap(const TILE_MAP* map);
int get_tile_scale(const TILE_MAP* map);

#endif // TILEMAP_H
