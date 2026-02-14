#include "room.h"
#include "entity.h"
#include "camera.h"
#include "layer.h"
#include "resource.h"
#include "input.h"
#include "media/music.h"
#include "media/font.h"
#include "media/animation.h"
#include "entities/hero.h"
#include "entities/coin.h"
#include "entities/particle_system.h"
#include "collision.h"
#include <algorithm>
#include <cmath>

Room::Room(Layer* bg, Layer* mid, Layer* fg, Camera* camera)
    : backgroundLayer_(bg)
    , middleLayer_(mid)
    , foregroundLayer_(fg)
    , camera_(camera)
{
    bgLayers_ = { mid, fg };
    midLayers_ = { fg };
    font_ = Resource::getFont("data/images/font.bmp");

    int w = getWidthInTiles();
    int h = getHeightInTiles();

    hookableArray_.resize(w);
    collidableArray_.resize(w);
    for (int x = 0; x < w; x++) {
        hookableArray_[x].resize(h);
        collidableArray_[x].resize(h);
        for (int y = 0; y < h; y++) {
            hookableArray_[x][y] = mid->getTile(x, y).hookable;
            collidableArray_[x][y] = mid->getTile(x, y).collidable;
        }
    }
}

Room::~Room() {
    for (Entity* e : entities_) {
        delete e;
    }
    for (Entity* e : entitiesToAdd_) {
        delete e;
    }
}

void Room::addEntity(Entity* entity) {
    Hero* h = dynamic_cast<Hero*>(entity);
    if (h && !hero_) {
        hero_ = h;
    }
    entity->setRoom(this);
    entitiesToAdd_.push_back(entity);
}

int Room::getWidthInTiles() const {
    return middleLayer_->width;
}

int Room::getHeightInTiles() const {
    return middleLayer_->height;
}

bool Room::isCollidable(int x, int y) const {
    if (x < 0 || x >= static_cast<int>(collidableArray_.size()) || y < 0) return false;
    if (y >= static_cast<int>(collidableArray_[x].size())) return false;
    if (x < destroyedToTileRow_) return false;
    return collidableArray_[x][y];
}

bool Room::isHookable(int x, int y) const {
    if (x < 0 || x >= static_cast<int>(hookableArray_.size()) || y < 0) return false;
    if (y >= static_cast<int>(hookableArray_[x].size())) return false;
    if (x < destroyedToTileRow_) return false;
    return hookableArray_[x][y];
}

void Room::setCollidable(int x, int y, bool v) {
    if (x < 0 || x >= static_cast<int>(collidableArray_.size()) || y < 0) return;
    if (y >= static_cast<int>(collidableArray_[x].size())) return;
    collidableArray_[x][y] = v;
}

void Room::setHookable(int x, int y, bool v) {
    if (x < 0 || x >= static_cast<int>(hookableArray_.size()) || y < 0) return;
    if (y >= static_cast<int>(hookableArray_[x].size())) return;
    hookableArray_[x][y] = v;
}

bool Room::isCompleted() const {
    if (isCompletedFlag_ && frameCounter_ > 240) {
        Music::stop();
        Input::enable();
        return true;
    }
    return false;
}

void Room::setCompleted() {
    for (Entity* e : entities_) {
        e->onLevelComplete();
    }
    frameCounter_ = 0;
    isCompletedFlag_ = true;
    hero_->imortal();
    Input::disable();
    Music::stop();
}

bool Room::damageDone(int x, int y) {
    for (Entity* e : damagableEntities_) {
        if (collidesRect(*e, static_cast<float>(x), static_cast<float>(y),
                         static_cast<float>(x + 1), static_cast<float>(y + 1))) {
            e->onDamage();
            return true;
        }
    }
    return false;
}

Entity* Room::findHookableEntity(Vec2 position) {
    for (Entity* e : hookableEntities_) {
        float dx = std::abs(e->getPosition().x - position.x);
        float dy = std::abs(e->getPosition().y - position.y);
        if (dx < e->getHalfSize().x && dy < e->getHalfSize().y) {
            return e;
        }
    }
    return nullptr;
}

bool Room::rayCast(Vec2 origin, Vec2 direction, bool cull, int& outX, int& outY) {
    Vec2 dir(direction);
    if (dir.isZero()) return false;

    int tw = getTileWidth();
    int th = getTileHeight();
    int ix = static_cast<int>(std::floor(std::floor(origin.x) / tw));
    int iy = static_cast<int>(std::floor(std::floor(origin.y) / th));
    float dx = origin.x / tw - ix;
    float dy = origin.y / th - iy;
    float tx = direction.x > 0 ? 1.0f : 0.0f;
    float ty = direction.y > 0 ? 1.0f : 0.0f;

    int minX = 0, maxX = getWidthInTiles();
    int minY = 0, maxY = getHeightInTiles();
    if (cull) {
        minX = std::max(minX, ix + static_cast<int>(std::floor(std::min(0.0f, direction.x) / tw - 1)));
        maxX = std::min(maxX, ix + static_cast<int>(std::floor(std::max(0.0f, direction.x) / tw + 2)));
        minY = std::max(minY, iy + static_cast<int>(std::floor(std::min(0.0f, direction.y) / th - 1)));
        maxY = std::min(maxY, iy + static_cast<int>(std::floor(std::max(0.0f, direction.y) / th + 2)));
    }

    while (ix >= minX && ix < maxX && iy >= minY && iy < maxY) {
        if (isCollidable(ix, iy)) {
            outX = ix;
            outY = iy;
            if (cull) {
                float cx = ix * tw + tw / 2.0f;
                float cy = iy * th + th / 2.0f;
                Vec2 v(origin.x - cx, origin.y - cy);
                if (v.lengthCompare(direction) > 0) return false;
            }
            return true;
        }

        if (direction.y == 0.0f || (direction.x != 0.0f && (tx - dx) / direction.x < (ty - dy) / direction.y)) {
            dy += direction.y * (tx - dx) / direction.x;
            if (direction.x > 0) { ix++; dx = 0; } else { ix--; dx = 1; }
        } else {
            dx += direction.x * (ty - dy) / direction.y;
            if (direction.y > 0) { iy++; dy = 0; } else { iy--; dy = 1; }
        }
    }
    return false;
}

void Room::broadcastButtonDown(int id) {
    for (Entity* e : entities_) e->onButtonDown(id);
}

void Room::broadcastButtonUp(int id) {
    for (Entity* e : entities_) e->onButtonUp(id);
}

void Room::broadcastStartWallOfDeath() {
    for (Entity* e : entities_) e->onStartWallOfDeath();
}

void Room::broadcastBossWallActivate() {
    for (Entity* e : entities_) e->onBossWallActivate();
}

void Room::broadcastBossWallDeactivate() {
    for (Entity* e : entities_) e->onBossWallDeactivate();
}

void Room::broadcastBossFloorActivate() {
    for (Entity* e : entities_) e->onBossFloorActivate();
}

void Room::destroyToTileRow(int x) {
    destroyedToTileRow_ = x;
    for (int y = 0; y < middleLayer_->height; y++) {
        if (middleLayer_->getTile(x, y).collidable) {
            ParticleSystem* ps = new ParticleSystem(
                Resource::getAnimation("data/images/debris.bmp", 4),
                20, 30, 10, 15, 40, 3, Vec2(0, -50), 5);
            ps->setPosition(Vec2(
                x * getTileWidth() + getTileWidth() * 0.75f,
                y * getTileHeight() + getTileHeight() * 0.75f));
            addEntity(ps);
        }
    }
    middleLayer_->setDestroyedToTileRow(x);
    foregroundLayer_->setDestroyedToTileRow(x);
}

void Room::onDraw(SDL_Renderer* renderer) {
    int ox = camera_->getOffsetX();
    int oy = camera_->getOffsetY();

    if (hero_->isDead()) {
        int fc = frameCounter_;
        if (fc == 0 || fc == 5 || fc == 10) {
            SDL_SetRenderDrawColor(renderer, 231, 215, 156, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 57, 56, 41, 255);
        }
        SDL_Rect rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderFillRect(renderer, &rect);
        if (fc < 60) {
            hero_->draw(renderer, ox, oy, 0);
        }
        return;
    }

    backgroundLayer_->draw(renderer, ox, oy, bgLayers_);
    middleLayer_->draw(renderer, ox, oy, midLayers_);

    for (Entity* e : entities_) {
        e->draw(renderer, ox, oy, 0);
    }

    foregroundLayer_->draw(renderer, ox, oy, fgLayers_);

    if (isCompletedFlag_) {
        font_->drawCenter(renderer, "LEVEL COMPLETED!", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    }
}

void Room::onLogic() {
    frameCounter_++;

    if (!entitiesToAdd_.empty()) {
        for (Entity* e : entitiesToAdd_) {
            if (e->isDamagable()) damagableEntities_.push_back(e);
            if (e->isHookable()) hookableEntities_.push_back(e);
            Coin* coin = dynamic_cast<Coin*>(e);
            if (coin && !coin->isDynamic()) staticCoins_.push_back(coin);
        }
        entities_.insert(entities_.end(), entitiesToAdd_.begin(), entitiesToAdd_.end());
        std::sort(entities_.begin(), entities_.end(), [](const Entity* a, const Entity* b) {
            if (a->getLayer() != b->getLayer()) return a->getLayer() < b->getLayer();
            return false;
        });
        entitiesToAdd_.clear();
    }

    if (isCompletedFlag_ && frameCounter_ == 60) {
        Music::playSong("data/music/level_completed.xm");
    }

    if (hero_->isDead() && !heroIsDead_) {
        frameCounter_ = 0;
        heroIsDead_ = true;
        return;
    }

    if (hero_->isDead() && frameCounter_ > 120) {
        hero_->respawn();
        for (Entity* e : entities_) e->onRespawn();
        camera_->centerToHero(*hero_);
        camera_->onRespawn();
        heroIsDead_ = false;
    }

    if (hero_->isDead()) return;

    for (int i = static_cast<int>(entities_.size()) - 1; i >= 0; i--) {
        if (entities_[i]->isRemoved()) {
            Entity* e = entities_[i];
            entities_.erase(entities_.begin() + i);
            {
                auto it = std::find(hookableEntities_.begin(), hookableEntities_.end(), e);
                if (it != hookableEntities_.end()) hookableEntities_.erase(it);
            }
            {
                auto it = std::find(damagableEntities_.begin(), damagableEntities_.end(), e);
                if (it != damagableEntities_.end()) damagableEntities_.erase(it);
            }
            delete e;
        }
    }

    for (int i = 0; i < static_cast<int>(entities_.size()); i++) {
        Entity* e = entities_[i];
        if (hero_->isDead()) break;
        if (e != static_cast<Entity*>(hero_)) e->update();
    }

    hero_->update();

    for (int i = static_cast<int>(staticCoins_.size()) - 1; i >= 0; i--) {
        Coin* coin = staticCoins_[i];
        if (coin->isRemoved()) {
            staticCoins_.erase(staticCoins_.begin() + i);
            continue;
        }
        if (coin->Collides(*hero_)) {
            coin->setCollides();
        }
    }

    float maxW = static_cast<float>(getWidthInTiles() * getTileWidth());
    float maxH = static_cast<float>(getHeightInTiles() * getTileHeight());
    for (Entity* e : entities_) {
        Vec2 p = e->getPosition();
        Vec2 hs = e->getHalfSize();
        if (p.x - hs.x > maxW || p.y - hs.y > maxH || p.x + hs.x < 0 || p.y + hs.y < 0) {
            e->remove();
        }
    }

    camera_->update(*hero_);
}
