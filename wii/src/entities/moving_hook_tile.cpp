#include "entities/moving_hook_tile.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "room.h"

MovingHookTile::MovingHookTile() {
    sprite_ = Resource::getAnimation("data/images/movinghooktile.bmp", 2);
    setSize(Vec2(static_cast<float>(sprite_->getFrameHeight()),
                 static_cast<float>(sprite_->getFrameHeight())));
    velocity_ = Vec2(20, 0);
}

void MovingHookTile::setRoom(Room* room) {
    initialPosition_ = position_;
    initialVelocity_ = velocity_;
    Entity::setRoom(room);
}

void MovingHookTile::onRespawn() {
    setPosition(initialPosition_);
    velocity_ = initialVelocity_;
    hasHook_ = false;
}

void MovingHookTile::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = getDrawPositionX() + offsetX - sprite_->getFrameWidth() / 2;
    int y = getDrawPositionY() + offsetY - sprite_->getFrameHeight() / 2;
    sprite_->drawFrame(renderer, frameCounter_ / 12, x, y);
}

void MovingHookTile::update() {
    Entity::update();

    hasHook_ = (room_->getHero()->getHookedEntity() == this);

    if (hasHook_) {
        velocity_.x = (velocity_.x > 0) ? 40.0f : -40.0f;
        frameCounter_++;
    } else {
        velocity_.x = (velocity_.x > 0) ? 20.0f : -20.0f;
    }

    auto bumps = moveWithCollision();
    if (bumps.count(Direction::LEFT) || bumps.count(Direction::RIGHT)) {
        velocity_.x = -velocity_.x;
    }
}
