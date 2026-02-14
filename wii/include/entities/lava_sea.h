#pragma once
#include "entity.h"

class Animation;

class LavaSea : public Entity {
public:
    LavaSea();

    int getLayer() const override { return 4; }

    float getCollideTop() const override { return getCurrentY(); }
    float getCollideLeft() const override { return 0; }
    float getCollideRight() const override { return 100000; }
    float getCollideBottom() const override { return 100000; }

    void onLevelComplete() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    float getCurrentY() const;

    int frame_ = 0;
    bool safeMode_ = false;
    float safeLevel_ = 0;
    Animation* topAnim_;
    Animation* fillAnim_;
};
