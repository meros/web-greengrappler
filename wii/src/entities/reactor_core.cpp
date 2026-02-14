#include "entities/reactor_core.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "room.h"
#include "player_skill.h"
#include <cmath>

ReactorCore::ReactorCore() {
    setSize(Vec2(30, 30));
    animation_ = Resource::getAnimation("data/images/core.bmp", 3);
}

void ReactorCore::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = static_cast<int>(std::round(position_.x - animation_->getFrameWidth() / 2.0f + offsetX));
    int y = static_cast<int>(std::round(position_.y - animation_->getFrameHeight() / 2.0f + offsetY));
    animation_->drawFrame(renderer, frame_ / 5, x, y);
}

void ReactorCore::update() {
    velocity_.y += 6.0f;
    auto bumps = moveWithCollision();
    if (bumps.count(Direction::UP) || bumps.count(Direction::DOWN)) {
        velocity_.y *= 0.8f;
    }

    if (room_->getHero()->Collides(*this)) {
        if (room_->getHero()->gotCore()) {
            PlayerSkill::playerDidSomethingClever(1.0f, 0.75f);
            room_->setCompleted();
            remove();
        }
    }
    frame_++;
}
