#include "entities/breaking_hook_tile.h"
#include "entities/particle_system.h"
#include "entities/hero.h"
#include "resource.h"
#include "media/animation.h"
#include "room.h"
#include "player_skill.h"
#include "utils.h"
#include <cmath>

static const int MAX_BREAK_FRAMES = 50;
static const int MIN_BREAK_FRAMES = 30;

BreakingHookTile::BreakingHookTile() {
    sprite_ = Resource::getAnimation("data/images/breakinghooktile.bmp");
    setSize(Vec2(static_cast<float>(sprite_->getFrameWidth()),
                 static_cast<float>(sprite_->getFrameHeight())));
}

void BreakingHookTile::setRoom(Room* room) {
    Entity::setRoom(room);
    onRespawn();
}

void BreakingHookTile::onRespawn() {
    tileX_ = static_cast<int>(std::floor(position_.x / room_->getTileWidth()));
    tileY_ = static_cast<int>(std::floor(position_.y / room_->getTileHeight()));
    room_->setHookable(tileX_, tileY_, true);
    room_->setCollidable(tileX_, tileY_, true);
    breakCounter_ = 0;
    breaking_ = false;
    destroyed_ = false;
}

void BreakingHookTile::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    if (destroyed_) return;
    int x = getDrawPositionX() + offsetX - sprite_->getFrameWidth() / 2;
    int y = getDrawPositionY() + offsetY - sprite_->getFrameWidth() / 2;
    if (!breaking_ || (breakCounter_ & 1) == 0) {
        sprite_->drawFrame(renderer, 0, x, y);
    }
}

void BreakingHookTile::update() {
    if (destroyed_) return;

    if (breaking_) {
        breakCounter_++;
        int breakFrames = lerpInt(static_cast<float>(MAX_BREAK_FRAMES),
                                   static_cast<float>(MIN_BREAK_FRAMES),
                                   PlayerSkill::get());

        Hero* hero = room_->getHero();
        if (hero->getRopeState() != RopeState::Attached) {
            breakFrames = -1;
        }

        if (breakCounter_ >= breakFrames) {
            destroyed_ = true;
            room_->setHookable(tileX_, tileY_, false);
            room_->setCollidable(tileX_, tileY_, false);

            ParticleSystem* ps = new ParticleSystem(
                Resource::getAnimation("data/images/debris.bmp", 4),
                10, 50, 20, 1.0f, 50.0f, 2, Vec2(0, -30), 2.0f);
            ps->setPositionWithSpread(position_, 2.0f, false);
            room_->addEntity(ps);
        }
    }

    Hero* hero = room_->getHero();
    if (hero->getRopeState() == RopeState::Attached) {
        Vec2 ropePos = hero->getRopePosition();
        int ropeTileX = static_cast<int>(std::floor(ropePos.x / room_->getTileWidth()));
        int ropeTileY = static_cast<int>(std::floor(ropePos.y / room_->getTileWidth()));
        if (ropeTileX == tileX_ && ropeTileY == tileY_) {
            breaking_ = true;
        }
    }
}
