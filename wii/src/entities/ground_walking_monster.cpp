#include "entities/ground_walking_monster.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "media/sound.h"
#include "room.h"
#include "player_skill.h"
#include "collision.h"
#include "utils.h"
#include <cmath>

static const float MIN_WALKING_SPEED = 40.0f;
static const float MAX_WALKING_SPEED = 70.0f;

GroundWalkingMonster::GroundWalkingMonster(GroundWalkingMonsterType type)
    : type_(type) {
    setSize(Vec2(10, 10));
    animation_ = Resource::getAnimation("data/images/saw.bmp", 2);
}

void GroundWalkingMonster::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = static_cast<int>(std::round(position_.x)) - animation_->getFrameWidth() / 2 + offsetX;
    int y = static_cast<int>(std::round(position_.y)) - animation_->getFrameHeight() / 2 + offsetY;
    animation_->drawFrame(renderer, frame_ / 3, x, y);
}

void GroundWalkingMonster::update() {
    Hero* hero = room_->getHero();
    float speed = lerp(MIN_WALKING_SPEED, MAX_WALKING_SPEED, PlayerSkill::get());

    if (type_ == GroundWalkingMonsterType::FLOOR || type_ == GroundWalkingMonsterType::ROOF) {
        if (facing_ == Facing::LEFT_UP) velocity_ = Vec2(-speed, 0);
        else velocity_ = Vec2(speed, 0);
    } else {
        if (facing_ == Facing::LEFT_UP) velocity_ = Vec2(0, -speed);
        else velocity_ = Vec2(0, speed);
    }

    auto bumps = moveWithCollision();

    if (bumps.count(Direction::LEFT) || bumps.count(Direction::UP))
        facing_ = Facing::RIGHT_DOWN;
    if (bumps.count(Direction::RIGHT) || bumps.count(Direction::DOWN))
        facing_ = Facing::LEFT_UP;

    float offsetX, offsetY;

    if (type_ == GroundWalkingMonsterType::FLOOR || type_ == GroundWalkingMonsterType::ROOF) {
        offsetX = (facing_ == Facing::LEFT_UP) ? -halfSize_.x - 2 : halfSize_.x + 2;
        offsetY = (type_ == GroundWalkingMonsterType::FLOOR) ? halfSize_.y + 2 : -halfSize_.y - 2;
    } else {
        offsetX = (type_ == GroundWalkingMonsterType::LEFT_WALL) ? -halfSize_.x - 2 : halfSize_.x + 2;
        offsetY = (facing_ == Facing::RIGHT_DOWN) ? halfSize_.y + 2 : -halfSize_.y - 2;
    }

    int tx = static_cast<int>(std::floor((position_.x + offsetX) / room_->getTileWidth()));
    int ty = static_cast<int>(std::floor((position_.y + offsetY) / room_->getTileHeight()));

    if (!room_->isCollidable(tx, ty)) {
        facing_ = (facing_ == Facing::LEFT_UP) ? Facing::RIGHT_DOWN : Facing::LEFT_UP;
    }

    if (hero->Collides(*this)) hero->kill();

    if (hero->hasHook()) {
        SimpleCollidable hookCol = hero->getHookCollidable();
        if (collides(*this, hookCol)) {
            Sound::playSample("data/sounds/hook");
            hero->detachHook();
        }
    }

    frame_++;
}
