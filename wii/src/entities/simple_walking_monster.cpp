#include "entities/simple_walking_monster.h"
#include "entities/particle_system.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "media/sound.h"
#include "room.h"
#include "player_skill.h"
#include <cmath>
#include <cstdlib>

static const float IDLE_TO_WALK_CHANCE = 0.05f;
static const float WALK_TO_IDLE_CHANCE = 0.01f;

SimpleWalkingMonster::SimpleWalkingMonster() {
    setSize(Vec2(20, 20));
    animation_ = Resource::getAnimation("data/images/robot.bmp", 4);
}

void SimpleWalkingMonster::die() {
    Sound::playSample("data/sounds/damage");
    ParticleSystem* ps = new ParticleSystem(
        Resource::getAnimation("data/images/debris.bmp", 4),
        10, 30, 10, 1.0f, 50.0f, 5, Vec2(0, -30), 2.0f);
    ps->setPositionWithSpread(position_, 5.0f, false);
    room_->addEntity(ps);
    remove();
}

void SimpleWalkingMonster::onDamage() {
    PlayerSkill::playerDidSomethingClever(0.5f, 0.1f);
    die();
}

void SimpleWalkingMonster::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = static_cast<int>(std::round(position_.x)) - animation_->getFrameWidth() / 2 + offsetX;
    int y = static_cast<int>(std::round(position_.y)) - animation_->getFrameHeight() / 2 + offsetY;
    animation_->drawFrame(renderer, frame_ / 15, x, y,
                          facing_ == Facing::RIGHT, false);
}

void SimpleWalkingMonster::update() {
    switch (state_) {
    case MonsterState::WALKING: {
        velocity_.x = 20.0f * (facing_ == Facing::LEFT ? -1.0f : 1.0f);
        velocity_.y += 6.0f;
        auto bumps = moveWithCollision();

        if (bumps.count(Direction::UP) || bumps.count(Direction::DOWN))
            velocity_.y = 0;
        if (bumps.count(Direction::LEFT)) facing_ = Facing::RIGHT;
        if (bumps.count(Direction::RIGHT)) facing_ = Facing::LEFT;

        float ox = (facing_ == Facing::RIGHT) ? -halfSize_.x - 2 : halfSize_.x + 2;
        float oy = halfSize_.y + 2;
        int tx = static_cast<int>(std::floor((position_.x + ox) / room_->getTileWidth()));
        int ty = static_cast<int>(std::floor((position_.y + oy) / room_->getTileHeight()));

        if (!room_->isCollidable(tx, ty)) {
            facing_ = (facing_ == Facing::LEFT) ? Facing::RIGHT : Facing::LEFT;
        }

        if ((static_cast<float>(rand()) / RAND_MAX) < WALK_TO_IDLE_CHANCE)
            state_ = MonsterState::IDLING;
        break;
    }
    case MonsterState::IDLING:
        if ((static_cast<float>(rand()) / RAND_MAX) < IDLE_TO_WALK_CHANCE) {
            facing_ = ((static_cast<float>(rand()) / RAND_MAX) > 0.5f) ? Facing::LEFT : Facing::RIGHT;
            state_ = MonsterState::WALKING;
        }
        break;
    }

    Hero* hero = room_->getHero();
    if (hero->Collides(*this)) hero->kill();
    frame_++;
}
