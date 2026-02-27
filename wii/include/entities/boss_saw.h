#pragma once
#include "entity.h"
#include "constants.h"

class Animation;

class BossSaw : public Entity {
public:
    BossSaw(Direction direction);

    int getLayer() const override { return 3; }

    void onRespawn() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    int lifeTime_ = 60 * 6;
    int sawFrameCounter_ = 0;
    Animation* saw_;
    Vec2 speed_;
};
