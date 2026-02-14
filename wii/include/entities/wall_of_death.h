#pragma once
#include "entity.h"

class Animation;

class WallOfDeath : public Entity {
public:
    WallOfDeath();

    int getLayer() const override { return 1; }

    void onRespawn() override;
    void onStartWallOfDeath() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    bool inited_ = false;
    bool boost_ = false;
    float speed_ = 0;
    int wallFrameCounter_ = 0;
    bool running_ = false;
    float soundCountdown_ = 10.0f;
    Animation* saw_;
    Vec2 originalPosition_;
};
