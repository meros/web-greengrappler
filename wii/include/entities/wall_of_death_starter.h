#pragma once
#include "entity.h"

class WallOfDeathStarter : public Entity {
public:
    WallOfDeathStarter();

    int getLayer() const override { return 1; }

    void onRespawn() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    bool used_ = false;
};
