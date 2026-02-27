#pragma once
#include "tile.h"
#include "constants.h"
#include <vector>

class Layer {
public:
    int width;
    int height;

    Layer(int w, int h);

    void setTile(int x, int y, const Tile& tile);
    const Tile& getTile(int x, int y) const;
    void setDestroyedToTileRow(int x);

    void draw(SDL_Renderer* renderer, int offsetX, int offsetY,
              const std::vector<Layer*>& overLayers) const;

private:
    std::vector<std::vector<Tile>> tiles_;
    int destroyedToTileRow_ = -1;
    static Tile emptyTile_;
};
