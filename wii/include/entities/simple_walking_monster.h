#pragma once
#include "entity.h"

class Animation;

class SimpleWalkingMonster : public Entity {
public:
    SimpleWalkingMonster();

    int getLayer() const override { return 1; }
    bool isDamagable() const override { return true; }

    void onDamage() override;
    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    enum class Facing { LEFT, RIGHT };
    enum class MonsterState { IDLING, WALKING };

    void die();

    Animation* animation_;
    Facing facing_ = Facing::LEFT;
    int frame_ = 0;
    MonsterState state_ = MonsterState::IDLING;
};
