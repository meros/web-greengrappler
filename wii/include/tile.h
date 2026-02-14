#pragma once
#include "resource.h"

struct Tile {
    GameImage image;
    bool hookable = false;
    bool collidable = false;
    bool hasImage = false;

    Tile() = default;
    Tile(GameImage tilemap, int x, int y, int w, int h, bool hook, bool collide)
        : image(tilemap.subImage(x, y, w, h)), hookable(hook), collidable(collide), hasImage(true) {}

    bool isOpaque() const { return hasImage; }
};
