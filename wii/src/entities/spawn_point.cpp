#include "entities/spawn_point.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "room.h"
#include "player_skill.h"
#include <cmath>

SpawnPoint::SpawnPoint() {
    setSize(Vec2(22, 25));
    animation_ = Resource::getAnimation("data/images/checkpoint.bmp", 3);
}

void SpawnPoint::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = static_cast<int>(std::round(position_.x)) - animation_->getFrameWidth() / 2 + offsetX;
    int y = static_cast<int>(std::round(position_.y)) - animation_->getFrameHeight() / 2 + offsetY;
    animation_->drawFrame(renderer, static_cast<int>(state_), x, y);
}

void SpawnPoint::update() {
    Hero* hero = room_->getHero();
    if (hero->Collides(*this)) {
        if (state_ == SpawnState::UNCHECKED) {
            PlayerSkill::playerDidSomethingClever(0.7f, 0.5f);
        }
        state_ = SpawnState::SEMI_CHECKED;
        hero->setLastSpawnPoint(position_);
    }

    if (state_ == SpawnState::SEMI_CHECKED) frame_++;
    if (frame_ > 5) state_ = SpawnState::CHECKED;
}
