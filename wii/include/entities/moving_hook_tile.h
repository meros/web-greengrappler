#pragma once
#include "entity.h"

class Animation;
class Room;

class MovingHookTile : public Entity {
public:
    MovingHookTile();

    int getLayer() const override { return 0; }
    bool isHookable() const override { return true; }

    void setRoom(Room* room) override;
    void onRespawn() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    Animation* sprite_;
    bool hasHook_ = false;
    Vec2 initialPosition_;
    Vec2 initialVelocity_;
};
