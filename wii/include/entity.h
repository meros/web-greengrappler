#pragma once
#include "constants.h"
#include "vec2.h"
#include "collision.h"
#include <set>
#include <cmath>
#include <SDL2/SDL.h>

class Room;

class Entity : public Collidable {
public:
    virtual ~Entity() = default;
    virtual int getLayer() const = 0;

    Vec2 getPosition() const { return position_; }
    Vec2 getVelocity() const { return velocity_; }
    Vec2 getSize() const { return size_; }
    Vec2 getHalfSize() const { return halfSize_; }
    int getDrawPositionX() const { return static_cast<int>(std::round(position_.x)); }
    int getDrawPositionY() const { return static_cast<int>(std::round(position_.y)); }

    void setPosition(Vec2 pos) { position_ = pos; }
    void setVelocity(Vec2 vel) { velocity_ = vel; }
    void setSize(Vec2 s) { size_ = s; halfSize_ = s * 0.5f; }
    void setRoom(Room* room) { room_ = room; }
    Room* getRoom() const { return room_; }

    bool isRemoved() const { return removed_; }
    virtual void remove() { removed_ = true; }

    virtual bool isDamagable() const { return false; }
    virtual bool isHookable() const { return false; }

    float getCollideTop() const override { return position_.y - halfSize_.y; }
    float getCollideLeft() const override { return position_.x - halfSize_.x; }
    float getCollideBottom() const override { return position_.y + halfSize_.y; }
    float getCollideRight() const override { return position_.x + halfSize_.x; }

    bool Collides(const Collidable& other) const { return collides(*this, other); }

    virtual void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) { (void)renderer; (void)offsetX; (void)offsetY; (void)layer; }
    virtual void update();

    std::set<Direction> moveWithCollision(float dx = -99999.0f, float dy = -99999.0f);

    // Override hooks
    virtual void onBossFloorActivate() {}
    virtual void onBossWallActivate() {}
    virtual void onBossWallDeactivate() {}
    virtual void onButtonDown(int id) { (void)id; }
    virtual void onButtonUp(int id) { (void)id; }
    virtual void onDamage() {}
    virtual void onLevelComplete() {}
    virtual void onRespawn() {}
    virtual void onStartWallOfDeath() {}

protected:
    int frameCounter_ = 0;
    bool removed_ = false;
    Room* room_ = nullptr;
    Vec2 position_;
    Vec2 velocity_;
    Vec2 size_;
    Vec2 halfSize_;
};
