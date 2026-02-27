#pragma once
#include "entity.h"

class Animation;

class SpawnPoint : public Entity {
public:
    SpawnPoint();

    int getLayer() const override { return 3; }

    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    enum class SpawnState { UNCHECKED = 0, SEMI_CHECKED = 1, CHECKED = 2 };

    SpawnState state_ = SpawnState::UNCHECKED;
    int frame_ = 0;
    Animation* animation_;
};
