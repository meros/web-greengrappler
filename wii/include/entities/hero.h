#pragma once
#include "entity.h"
#include "vec2.h"
#include <set>

class Animation;
class ParticleSystem;

enum class MovementState { AirRun, Fall, Jump, Run, Still };
enum class RopeState { Attached, Disappearing, Moving, Retracted };

class Hero : public Entity {
public:
    Hero();
    int getLayer() const override { return 2; }

    Vec2 getRopePosition() const { return ropePosition_; }
    RopeState getRopeState() const { return ropeState_; }
    bool gotCoin();
    bool gotCore();
    void imortal() { isImmortal_ = true; }
    bool isDead() const { return dead_; }
    bool hasHook() const { return ropeState_ == RopeState::Attached; }
    Entity* getHookedEntity() const { return hookedEntity_; }
    void detachHook();
    void setLastSpawnPoint(Vec2 sp) { spawnPoint_ = sp; }
    void kill();
    void remove() override;
    void respawn();
    SimpleCollidable getHookCollidable() const;

    void draw(SDL_Renderer* renderer, int offsetX, int offsetY, int layer) override;
    void update() override;

private:
    void die();
    float getAutoAimScore(Vec2 ropeDir, Vec2 aimPos) const;
    void adjustRopeDirection(Vec2& ropeDir);

    Animation* animFall_;
    Animation* animHook_;
    Animation* animHookParticle_;
    Animation* animHurt_;
    Animation* animJump_;
    Animation* animRope_;
    Animation* animRun_;

    int blinkingTicksLeft_ = 0;
    Direction facingDirection_ = Direction::RIGHT;
    int frame_ = 0;
    Entity* hookedEntity_ = nullptr;
    Vec2 hookedEntityOffset_;
    bool jumpHeld_ = false;
    bool jumpPrepressed_ = false;
    MovementState movementState_ = MovementState::Still;
    bool onGround_ = false;
    int ropeDisappearCounter_ = 0;
    float ropeMaxLength_ = 180.0f;
    Vec2 ropePosition_;
    RopeState ropeState_ = RopeState::Retracted;
    Vec2 ropeVelocity_;
    Vec2 spawnPoint_ = {-200, -200};
    bool isImmortal_ = false;
    bool dead_ = false;

    static constexpr float AIR_ACCELERATION = 4.5f;
    static constexpr float AIR_DRAG = 0.98f;
    static constexpr int BLINKING_TICKS = 100;
    static constexpr float GRAVITY = 6.0f;
    static constexpr float GROUND_ACCELERATION = 6.5f;
    static constexpr float GROUND_DRAG = 0.97f;
    static constexpr float GROUND_STOP_VELOCITY = 4.5f;
    static constexpr float JUMP_GRAVITY = 3.0f;
    static constexpr float JUMP_VELOCITY = 160.0f;
    static constexpr float KILL_VELOCITY = 200.0f;
    static constexpr float MAX_DEATH_COIN_LIFETIME = 500.0f;
    static constexpr float MIN_DEATH_COIN_LIFETIME = 300.0f;
    static constexpr int ROPE_DISAPPEAR_TICKS = 6;
    static constexpr float ROPE_MAX_ACCELERATION = 12.0f;
    static constexpr float ROPE_REST_LENGTH = 45.0f;
    static constexpr float ROPE_SPEED = 700.0f;
    static constexpr float ROPE_SPRING_CONSTANT = 0.6f;
};
