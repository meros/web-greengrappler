#include "layer.h"
#include <cmath>

Tile Layer::emptyTile_;

Layer::Layer(int w, int h) : width(w), height(h) {
    tiles_.resize(w);
    for (int x = 0; x < w; x++) {
        tiles_[x].resize(h);
    }
}

void Layer::setTile(int x, int y, const Tile& tile) {
    if (x >= 0 && x < width && y >= 0 && y < height)
        tiles_[x][y] = tile;
}

const Tile& Layer::getTile(int x, int y) const {
    if (x <= destroyedToTileRow_ || y < 0 || x >= width || y >= height)
        return emptyTile_;
    return tiles_[x][y];
}

void Layer::setDestroyedToTileRow(int x) {
    destroyedToTileRow_ = x;
}

void Layer::draw(SDL_Renderer* renderer, int offsetX, int offsetY,
                 const std::vector<Layer*>& overLayers) const {
    int firstTileX = static_cast<int>(std::floor(static_cast<float>(-offsetX) / TILE_SIZE));
    int lastTileX = static_cast<int>(std::floor(static_cast<float>(SCREEN_WIDTH - offsetX) / TILE_SIZE));
    int firstTileY = static_cast<int>(std::floor(static_cast<float>(-offsetY) / TILE_SIZE));
    int lastTileY = static_cast<int>(std::floor(static_cast<float>(SCREEN_HEIGHT - offsetY) / TILE_SIZE));

    for (int y = firstTileY; y <= lastTileY; y++) {
        for (int x = firstTileX; x <= lastTileX + 1; x++) {
            const Tile& tile = getTile(x, y);

            bool dontDraw = false;
            for (auto* overLayer : overLayers) {
                if (overLayer->getTile(x, y).isOpaque()) { dontDraw = true; break; }
            }
            if (dontDraw) continue;

            if (tile.hasImage) {
                tile.image.draw(renderer, x * TILE_SIZE + offsetX, y * TILE_SIZE + offsetY);
            }
        }
    }
}
