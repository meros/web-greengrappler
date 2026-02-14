#pragma once
#include "entity.h"
#include "resource.h"

class Spike : public Entity {
public:
    Spike();

    int getLayer() const override { return 3; }

    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    GameImage spikeTile_;
};
