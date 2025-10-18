#ifndef TILEMAP_H
#define TILEMAP_H

#include "raylib.h"

#define TILE_SIZE 16
#define MAP_WIDTH 25
#define MAP_HEIGHT 20

typedef struct {
    int layer1[MAP_HEIGHT][MAP_WIDTH];
    int layer2[MAP_HEIGHT][MAP_WIDTH];
    texture_2d tileset1;  // For layer 1
    texture_2d tileset2;  // For layer 2
    int tile_size;
    int map_width;
    int map_height;
} tile_map;

// Function declarations
tile_map init_tilemap();
void draw_tilemap(const tile_map* map);
void unload_tilemap(const tile_map* map);
int get_tile_scale(const tile_map* map);
void draw_texture(const tile_map* map, texture_2d tileset, int tile_index, int x, int y);
#endif // TILEMAP_H
