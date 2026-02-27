#pragma once
#include "entity.h"

class Animation;

class ButtonEntity : public Entity {
public:
    ButtonEntity(int id);

    int getLayer() const override { return 0; }

    void onRespawn() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    bool triggered_ = false;
    int time_ = 60 * 5;
    int counter_;
    int id_;
    Animation* buttonAnim_;
};
