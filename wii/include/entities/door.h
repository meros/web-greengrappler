#pragma once
#include "entity.h"

class Animation;

class Door : public Entity {
public:
    Door(int id);

    int getLayer() const override { return 0; }

    void onRespawn() override;
    void onButtonDown(int id) override;
    void onButtonUp(int id) override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    int id_;
    bool opening_ = false;
    bool closing_ = false;
    int doorHeight_ = 40;
    int doorFrameCounter_ = 0;
    Animation* doorAnim_;
};
