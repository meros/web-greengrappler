#pragma once
#include "entity.h"

class Animation;
class Room;

class BreakingHookTile : public Entity {
public:
    BreakingHookTile();

    int getLayer() const override { return 3; }

    void setRoom(Room* room) override;
    void onRespawn() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    int breakCounter_ = 0;
    bool breaking_ = false;
    bool destroyed_ = false;
    Animation* sprite_;
    int tileX_ = 0;
    int tileY_ = 0;
};
