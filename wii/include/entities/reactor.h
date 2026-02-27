#pragma once
#include "entity.h"

class Animation;
class Room;

class Reactor : public Entity {
public:
    Reactor();

    int getLayer() const override { return 1; }
    bool isDamagable() const override { return true; }

    float getCollideTop() const override { return position_.y - halfSize_.y - 2; }
    float getCollideLeft() const override { return position_.x - halfSize_.x - 2; }
    float getCollideBottom() const override { return position_.y + halfSize_.y + 2; }
    float getCollideRight() const override { return position_.x + halfSize_.x + 2; }

    void setRoom(Room* room) override;
    void onDamage() override;
    void onRespawn() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    void setTilesCollidable(bool v);

    bool aboutToBlow_ = false;
    int damage_ = 0;
    int reactorFrameCounter_ = 0;
    Animation* shell_;
};
