#pragma once
#include "entity.h"

class Animation;

enum class GroundWalkingMonsterType { FLOOR, LEFT_WALL, RIGHT_WALL, ROOF };

class GroundWalkingMonster : public Entity {
public:
    GroundWalkingMonster(GroundWalkingMonsterType type);

    int getLayer() const override { return 1; }

    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    enum class Facing { LEFT_UP, RIGHT_DOWN };

    Animation* animation_;
    Facing facing_ = Facing::RIGHT_DOWN;
    int frame_ = 0;
    GroundWalkingMonsterType type_;
};
