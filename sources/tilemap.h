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
    int tileSize;
    int mapWidth;
    int mapHeight;
} Tilemap;

// Function declarations
Tilemap InitTilemap(void);
void DrawTilemap(Tilemap* map);
void UnloadTilemap(const Tilemap* map);

#endif // TILEMAP_H
