#pragma once
#include "entity.h"

class Animation;

class ReactorCore : public Entity {
public:
    ReactorCore();

    int getLayer() const override { return 1; }

    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    Animation* animation_;
    int frame_ = 0;
};
