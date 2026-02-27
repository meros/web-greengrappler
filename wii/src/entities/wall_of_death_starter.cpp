#include "entities/wall_of_death_starter.h"
#include "entities/hero.h"
#include "room.h"

WallOfDeathStarter::WallOfDeathStarter() {
    setSize(Vec2(10, 10 * 100));
}

void WallOfDeathStarter::onRespawn() {
    used_ = false;
}

void WallOfDeathStarter::draw(SDL_Renderer* /*renderer*/, int /*offsetX*/, int /*offsetY*/, int /*layer*/) {
    // Invisible entity
}

void WallOfDeathStarter::update() {
    if (!used_ && room_->getHero()->Collides(*this)) {
        room_->broadcastStartWallOfDeath();
        used_ = true;
    }
}
