#pragma once
#include "entity.h"

class BossFloor : public Entity {
public:
    BossFloor();

    int getLayer() const override { return 4; }

    float getCollideTop() const override { return Entity::getCollideTop() - 1; }

    void onBossFloorActivate() override;
    void onRespawn() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    bool active_ = false;
    int floorFrameCounter_ = 0;
};
