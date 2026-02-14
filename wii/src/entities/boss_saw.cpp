#include "entities/boss_saw.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "media/sound.h"
#include "room.h"
#include <cmath>

BossSaw::BossSaw(Direction direction) {
    speed_ = (direction == Direction::LEFT) ? Vec2(-2, 0) : Vec2(2, 0);
    Sound::playSample("data/sounds/boss_saw");
    saw_ = Resource::getAnimation("data/images/saw.bmp");
    setSize(Vec2(static_cast<float>(saw_->getFrameWidth()),
                 static_cast<float>(saw_->getFrameHeight())));
}

void BossSaw::onRespawn() {
    remove();
}

void BossSaw::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = static_cast<int>(std::round(getDrawPositionX() + offsetX - halfSize_.x));
    int y = static_cast<int>(std::round(getDrawPositionY() + offsetY - halfSize_.y));
    saw_->drawFrame(renderer, sawFrameCounter_ / 10, x, y);
}

void BossSaw::update() {
    sawFrameCounter_++;
    if (sawFrameCounter_ > lifeTime_) remove();
    position_ += speed_;
    if (room_->getHero()->Collides(*this)) room_->getHero()->kill();
}
