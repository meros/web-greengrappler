#include "entities/hero.h"
#include "room.h"
#include "input.h"
#include "game_state.h"
#include "player_skill.h"
#include "resource.h"
#include "media/sound.h"
#include "media/animation.h"
#include "entities/coin.h"
#include "entities/particle_system.h"
#include "collision.h"
#include "utils.h"
#include <cmath>
#include <algorithm>

Hero::Hero() {
    setSize({10, 15});
    animFall_ = Resource::getAnimation("data/images/hero_fall.bmp", 1);
    animHook_ = Resource::getAnimation("data/images/hook.bmp", 1);
    animHookParticle_ = Resource::getAnimation("data/images/particles.bmp");
    animHurt_ = Resource::getAnimation("data/images/hero_hurt.bmp", 1);
    animJump_ = Resource::getAnimation("data/images/hero_jump.bmp", 1);
    animRope_ = Resource::getAnimation("data/images/rope.bmp", 1);
    animRun_ = Resource::getAnimation("data/images/hero_run.bmp", 4);
}

bool Hero::gotCoin() {
    if (blinkingTicksLeft_ == 0) {
        GameState::put("coins", GameState::getInt("coins") + 1);
        return true;
    }
    return false;
}

bool Hero::gotCore() {
    if (blinkingTicksLeft_ == 0) {
        GameState::put("coins", GameState::getInt("coins") + 1);
        return true;
    }
    return false;
}

void Hero::detachHook() {
    if (ropeState_ != RopeState::Retracted && ropeState_ != RopeState::Disappearing) {
        ropeState_ = RopeState::Disappearing;
        ropeDisappearCounter_ = 0;
    }
    hookedEntity_ = nullptr;
}

void Hero::die() {
    ropeState_ = RopeState::Retracted;
    hookedEntity_ = nullptr;
    dead_ = true;
    Sound::playSample("data/sounds/start");
}

void Hero::kill() {
    if (isImmortal_) return;
    if (blinkingTicksLeft_ == 0) {
        PlayerSkill::playerDidSomethingStupid(0.0f, 0.1f);
        detachHook();

        int coins = GameState::getInt("coins");
        GameState::put("coins", 0);

        if (coins > 0) {
            coins = coins / 2;
            coins = std::min(20, std::max(0, coins));
            float lifetime = lerp(MAX_DEATH_COIN_LIFETIME, MIN_DEATH_COIN_LIFETIME, PlayerSkill::get());
            Coin::spawnDeathCoins(coins, getPosition(), lifetime, room_);
            blinkingTicksLeft_ = BLINKING_TICKS;
            Vec2 knockback = velocity_.normalized() * -KILL_VELOCITY;
            velocity_ = knockback;
            Sound::playSample("data/sounds/hurt");
        } else {
            PlayerSkill::playerDidSomethingStupid(0.0f, 0.25f);
            die();
        }
    }
}

void Hero::remove() {
    die();
}

void Hero::respawn() {
    setPosition(spawnPoint_);
    setVelocity({0, 0});
    dead_ = false;
    GameState::put("coins", 0);
    onGround_ = false;
    jumpHeld_ = false;
    jumpPrepressed_ = false;
    frame_ = 0;
    ropeState_ = RopeState::Retracted;
    hookedEntity_ = nullptr;
    facingDirection_ = Direction::RIGHT;
    blinkingTicksLeft_ = 0;
    ropeDisappearCounter_ = 0;
}

SimpleCollidable Hero::getHookCollidable() const {
    float rx = ropePosition_.x;
    float ry = ropePosition_.y;
    return SimpleCollidable(rx - 2, ry - 2, rx + 2, ry + 2);
}

float Hero::getAutoAimScore(Vec2 ropeDir, Vec2 aimPos) const {
    Vec2 playerToTile = aimPos - getPosition();
    float dotValue = ropeDir.dot(playerToTile);
    if (dotValue < 0) return -1.0f;
    float distance = playerToTile.length();
    dotValue /= distance;
    if (dotValue < 0.80f) return -1.0f;
    return std::pow(dotValue, 10.0f) / distance;
}

void Hero::adjustRopeDirection(Vec2& ropeDir) {
    float bestScore = 0.0f;
    Vec2 bestDirection = ropeDir;
    Room* room = room_;

    for (int y = 0; y < room->getHeightInTiles(); y++) {
        for (int x = 0; x < room->getWidthInTiles(); x++) {
            if (room->isHookable(x, y)) {
                float pixelX = static_cast<float>(x * room->getTileWidth() + room->getTileWidth() / 2);
                float pixelY = static_cast<float>(y * room->getTileHeight() + room->getTileHeight() / 2);
                Vec2 tilePos(pixelX, pixelY);
                float score = getAutoAimScore(ropeDir, tilePos);

                if (score > bestScore) {
                    Vec2 direction = tilePos - getPosition();
                    int rcX = 0;
                    int rcY = 0;
                    bool rcHit = room->rayCast(getPosition(), direction, false, rcX, rcY);
                    if (rcHit && rcX == x && rcY == y) {
                        bestScore = score;
                        bestDirection = direction;
                    }
                }
            }
        }
    }

    const auto& entities = room->getEntities();
    for (Entity* e : entities) {
        if (!e->isDamagable() && !e->isHookable()) continue;
        Vec2 entityPos = e->getPosition();
        float score = getAutoAimScore(ropeDir, entityPos);

        if (score > bestScore) {
            Vec2 direction = entityPos - getPosition();
            int rcX = 0;
            int rcY = 0;
            bool rcHit = room->rayCast(getPosition(), direction, false, rcX, rcY);
            Vec2 rcTilePos(
                static_cast<float>(rcX * room->getTileWidth() + room->getTileWidth() / 2),
                static_cast<float>(rcY * room->getTileHeight() + room->getTileHeight() / 2)
            );
            if (!rcHit || ropeDir.lengthCompare(rcTilePos) < 0) {
                bestScore = score;
                bestDirection = direction;
            }
        }
    }

    ropeDir = bestDirection.normalized();
}

void Hero::draw(SDL_Renderer* renderer, int offsetX, int offsetY, int /*layer*/) {
    int x = getDrawPositionX();
    int y = getDrawPositionY();

    if (dead_) {
        animHurt_->drawFrame(renderer, 0,
            offsetX + x - animHurt_->getFrameWidth() / 2,
            static_cast<int>(std::round(offsetY + y + getHalfSize().y - animHurt_->getFrameHeight())),
            facingDirection_ == Direction::LEFT, false);
        return;
    }

    Animation* animation = nullptr;
    int frame = 1;

    switch (movementState_) {
        case MovementState::Run:
            animation = animRun_;
            frame = frame_ / 10;
            break;
        case MovementState::AirRun:
            animation = animRun_;
            frame = frame_ / 3;
            break;
        case MovementState::Still:
            animation = animRun_;
            frame = 1;
            break;
        case MovementState::Jump:
            animation = animJump_;
            frame = 1;
            break;
        case MovementState::Fall:
            animation = animFall_;
            frame = 1;
            break;
    }

    // Draw rope
    if (ropeState_ != RopeState::Retracted &&
        (ropeState_ != RopeState::Disappearing || (ropeDisappearCounter_ & 1) != 0)) {
        float x2 = static_cast<float>(getDrawPositionX() + offsetX);
        float y2 = static_cast<float>(getDrawPositionY() + offsetY);
        float x1 = std::round(ropePosition_.x) + offsetX;
        float y1 = std::round(ropePosition_.y) + offsetY;

        Vec2 lineVec(x2 - x1, y2 - y1);
        float rlength = std::round(lineVec.length());
        lineVec.normalize();
        lineVec *= 3.0f;

        int segments = static_cast<int>(std::ceil(rlength / lineVec.length()));
        float rx = x1;
        float ry = y1;

        for (int i = 0; i < segments; i++) {
            float waveX = 0.0f;
            float waveY = 0.0f;

            if ((ropeState_ == RopeState::Moving || ropeState_ == RopeState::Disappearing) && segments > 1) {
                float t = static_cast<float>(i) / static_cast<float>(segments - 1);
                float value = std::sin(t * static_cast<float>(M_PI)) * std::sin(i * 0.3f);
                waveX = lineVec.y * value;
                waveY = -lineVec.x * value;
            }

            animRope_->drawFrame(renderer, frame,
                static_cast<int>(std::round(rx + waveX - animRope_->getFrameWidth() / 2)),
                static_cast<int>(std::round(ry + waveY - animRope_->getFrameHeight() / 2)));

            rx += lineVec.x;
            ry += lineVec.y;
        }

        animHook_->drawFrame(renderer, frame,
            static_cast<int>(x1) - animHook_->getFrameWidth() / 2,
            static_cast<int>(y1) - animHook_->getFrameHeight() / 2);
    }

    if (blinkingTicksLeft_ > 0 && !onGround_) {
        animation = animHurt_;
    }

    if ((blinkingTicksLeft_ / 6) % 2 == 0) {
        animation->drawFrame(renderer, frame,
            offsetX + x - animation->getFrameWidth() / 2,
            static_cast<int>(std::round(offsetY + y + getHalfSize().y - animation->getFrameHeight())),
            facingDirection_ == Direction::LEFT, false);
    }
}

void Hero::update() {
    if (dead_) return;
    Entity::update();

    if (spawnPoint_.x < -100 && spawnPoint_.y < -100) {
        spawnPoint_ = getPosition();
    }

    if (ropeState_ == RopeState::Disappearing) {
        ropeDisappearCounter_++;
        if (ropeDisappearCounter_ >= ROPE_DISAPPEAR_TICKS) {
            ropeState_ = RopeState::Retracted;
        }
    }

    float acceleration = onGround_ ? GROUND_ACCELERATION : AIR_ACCELERATION;
    bool airRunning = false;

    if (Input::isHeld(Button::LEFT)) {
        velocity_.x -= acceleration;
        if (ropeState_ == RopeState::Attached && !onGround_) {
            facingDirection_ = Direction::LEFT;
            airRunning = true;
            movementState_ = MovementState::AirRun;
        }
    }

    if (Input::isHeld(Button::RIGHT)) {
        velocity_.x += acceleration;
        if (ropeState_ == RopeState::Attached && !onGround_) {
            facingDirection_ = Direction::RIGHT;
            airRunning = true;
            movementState_ = MovementState::AirRun;
        }
    }

    if (Input::isPressed(Button::JUMP)) {
        if (!jumpPrepressed_ && onGround_) Sound::playSample("data/sounds/jump");
        jumpPrepressed_ = true;
    }

    if (onGround_ && jumpPrepressed_) {
        velocity_.y -= JUMP_VELOCITY;
        if (ropeState_ != RopeState::Attached) {
            jumpHeld_ = true;
        }
        jumpPrepressed_ = false;
    }

    if (Input::isReleased(Button::JUMP)) {
        if (jumpHeld_ && velocity_.y < 0) {
            velocity_.y = velocity_.y * 0.5f;
        }
        jumpHeld_ = false;
        jumpPrepressed_ = false;
    }

    if (Input::isReleased(Button::FIRE)) {
        detachHook();
    }

    if (velocity_.y >= 0) {
        jumpHeld_ = false;
    }

    if (!airRunning) {
        movementState_ = MovementState::Still;
    }

    if (onGround_) {
        if (velocity_.x > 0) facingDirection_ = Direction::RIGHT;
        else if (velocity_.x < 0) facingDirection_ = Direction::LEFT;
        if (std::abs(velocity_.x) > GROUND_STOP_VELOCITY) {
            movementState_ = MovementState::Run;
        }
    } else if (!airRunning) {
        movementState_ = velocity_.y > 0 ? MovementState::Jump : MovementState::Fall;
    }

    if (movementState_ == MovementState::Still) {
        velocity_.x = 0;
    }

    // Fire hook
    if (Input::isPressed(Button::FIRE)) {
        Sound::playSample("data/sounds/rope");
        ropeState_ = RopeState::Moving;
        ropePosition_ = getPosition();
        ropeVelocity_ = {0, 0};

        if (Input::isHeld(Button::LEFT)) ropeVelocity_.x -= 1;
        if (Input::isHeld(Button::RIGHT)) ropeVelocity_.x += 1;
        if (Input::isHeld(Button::UP)) ropeVelocity_.y -= 1;
        if (Input::isHeld(Button::DOWN)) ropeVelocity_.y += 1;

        if (ropeVelocity_.isZero()) {
            ropeVelocity_.x = (facingDirection_ == Direction::LEFT) ? -1.0f : 1.0f;
        }

        ropeVelocity_ = (ropeVelocity_.normalized() * ROPE_SPEED + velocity_).normalized();
        adjustRopeDirection(ropeVelocity_);
        ropeVelocity_ *= ROPE_SPEED;

        if (ropeVelocity_.x < 0) facingDirection_ = Direction::LEFT;
        else if (ropeVelocity_.x > 0) facingDirection_ = Direction::RIGHT;
    }

    // Move rope
    if (ropeState_ == RopeState::Moving) {
        int substeps = 25;
        for (int i = 0; i < substeps; i++) {
            ropePosition_ += ropeVelocity_ / static_cast<float>(substeps * TICKS_PER_SECOND);
            int tileX = static_cast<int>(std::floor(ropePosition_.x / room_->getTileWidth()));
            int tileY = static_cast<int>(std::floor(ropePosition_.y / room_->getTileHeight()));

            if (room_->isHookable(tileX, tileY)) {
                Sound::playSample("data/sounds/hook");
                ropeState_ = RopeState::Attached;
                jumpHeld_ = false;

                ParticleSystem* ps = new ParticleSystem(
                    animHookParticle_, 2, 30, 10, 1, 50, 10,
                    ropeVelocity_.normalized() * -10.0f,
                    1.0f);
                ps->setPosition(ropePosition_);
                room_->addEntity(ps);
                break;
            }

            if ((ropePosition_ - getPosition()).length() > ropeMaxLength_) {
                detachHook();
                Sound::playSample("data/sounds/no_hook");
                break;
            }

            if (room_->isCollidable(tileX, tileY)) {
                detachHook();
                Sound::playSample("data/sounds/no_hook");
                break;
            }

            if (room_->damageDone(static_cast<int>(std::floor(ropePosition_.x)),
                                  static_cast<int>(std::floor(ropePosition_.y)))) {
                detachHook();
                break;
            }

            hookedEntity_ = room_->findHookableEntity(ropePosition_);
            if (hookedEntity_ != nullptr) {
                Sound::playSample("data/sounds/hook");
                ropeState_ = RopeState::Attached;
                jumpHeld_ = false;
                Vec2 off = ropePosition_ - hookedEntity_->getPosition();
                hookedEntityOffset_ = {std::floor(off.x), std::floor(off.y)};
            }
        }
    }

    // Attached rope physics
    if (ropeState_ == RopeState::Attached) {
        if (hookedEntity_ != nullptr) {
            ropePosition_ = hookedEntity_->getPosition() + hookedEntityOffset_;
        }

        Vec2 ropeToHero = getPosition() - ropePosition_;
        if (ropeToHero.lengthCompare(ROPE_REST_LENGTH) > 0) {
            Vec2 ropeRestPoint = ropePosition_ + ropeToHero.normalized() * ROPE_REST_LENGTH;
            Vec2 heroToRestPoint = ropeRestPoint - getPosition();
            Vec2 ropeAcceleration = heroToRestPoint * ROPE_SPRING_CONSTANT;
            if (ropeAcceleration.lengthCompare(ROPE_MAX_ACCELERATION) > 0) {
                ropeAcceleration = ropeAcceleration.normalized() * ROPE_MAX_ACCELERATION;
            }
            velocity_ += ropeAcceleration;
        }

        if (Input::isHeld(Button::UP)) velocity_.y -= acceleration;
        if (Input::isHeld(Button::DOWN)) velocity_.y += acceleration;

        int ropeTileX = static_cast<int>(std::floor(ropePosition_.x / room_->getTileWidth()));
        int ropeTileY = static_cast<int>(std::floor(ropePosition_.y / room_->getTileWidth()));
        if (hookedEntity_ == nullptr && !room_->isHookable(ropeTileX, ropeTileY)) {
            detachHook();
        }
    }

    std::set<Direction> bumps = moveWithCollision();

    if (bumps.count(Direction::LEFT) || bumps.count(Direction::RIGHT)) {
        velocity_.x = 0;
    }
    if (bumps.count(Direction::UP) || bumps.count(Direction::DOWN)) {
        velocity_.y = 0;
    }

    float gravity = jumpHeld_ ? JUMP_GRAVITY : GRAVITY;
    velocity_.y += gravity;
    bool ground = bumps.count(Direction::DOWN) > 0;
    if (ground && !onGround_ && ropeState_ != RopeState::Attached) {
        Sound::playSample("data/sounds/land");
    }
    onGround_ = ground;

    float drag = onGround_ ? GROUND_DRAG : AIR_DRAG;
    velocity_ *= drag;

    frame_++;

    if (blinkingTicksLeft_ > 0) {
        blinkingTicksLeft_--;
    }
}
