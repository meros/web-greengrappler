#pragma once
#include "entity.h"
#include "constants.h"

class Animation;
class Room;

class Boss : public Entity {
public:
    Boss();

    int getLayer() const override { return 3; }
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
    enum class BossState {
        INIT, SLEEPING, INITIAL_BLOW, AWAKENING, MOVE_UPWARDS, FLOAT, STOP, ATTACK, VULNERABLE, DEAD
    };

    void setTilesCollidable(bool v);
    void spawnDebris();

    float health_;
    BossState state_ = BossState::INIT;
    Direction direction_ = Direction::LEFT;
    int initialHealth_ = 16;
    int animFrameCounter_ = 0;
    Animation* reactorAnim_;
    Animation* bossAnim_;
    int bossFrameCounter_ = 0;
    Vec2 originalPos_;
};
