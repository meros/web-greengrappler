import { TICKS_PER_SECOND, Button, Direction } from '../constants.js';
import { Vec2, ImmutableVec2, type ReadonlyVec2 } from '../math.js';
import { lerp } from '../utils.js';
import { Entity } from '../entity.js';
import { Input } from '../input.js';
import { GameState } from '../gameState.js';
import { PlayerSkill } from '../playerSkill.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import type { Animation } from '../media/animation.js';
import type { Collidable } from '../collision.js';
import { ParticleSystem } from './particleSystem.js';
import { Coin } from './coin.js';

const enum MovementState { AirRun, Fall, Jump, Run, Still }
export const enum RopeState { Attached, Disappearing, Moving, Retracted }

const AIR_ACCELERATION = 4.5;
const AIR_DRAG = 0.98;
const BLINKING_TICKS = 100;
const GRAVITY = 6.0;
const GROUND_ACCELERATION = 6.5;
const GROUND_DRAG = 0.97;
const GROUND_STOP_VELOCITY = 4.5;
const JUMP_GRAVITY = 3.0;
const JUMP_VELOCITY = 160.0;
const KILL_VELOCITY = 200.0;
const MAX_DEATH_COIN_LIFETIME = 500;
const MIN_DEATH_COIN_LIFETIME = 300;
const ROPE_DISAPPEAR_TICKS = 6;
const ROPE_MAX_ACCELERATION = 12.0;
const ROPE_REST_LENGTH = 45.0;
const ROPE_SPEED = 700.0;
const ROPE_SPRING_CONSTANT = 0.6;

export class Hero extends Entity {
  private readonly animFall: Animation;
  private readonly animHook: Animation;
  private readonly animHookParticle: Animation;
  private readonly animHurt: Animation;
  private readonly animJump: Animation;
  private readonly animRope: Animation;
  private readonly animRun: Animation;

  private blinkingTicksLeft = 0;
  private facingDirection: Direction = Direction.RIGHT;
  private frame = 0;
  private hookedEntity: Entity | null = null;
  private hookedEntityOffset = new ImmutableVec2();
  private jumpHeld = false;
  private jumpPrepressed = false;
  private movementState: MovementState = MovementState.Still;
  private onGround = false;
  private ropeDisappearCounter = 0;
  private ropeMaxLength = 180;
  private ropePosition = new ImmutableVec2();
  private ropeState: RopeState = RopeState.Retracted;
  private readonly ropeVelocity = new Vec2();
  private spawnPoint = new ImmutableVec2(-200, -200);
  private isImmortal = false;
  private dead = false;

  constructor() {
    super();
    this.setSize(new ImmutableVec2(10, 15));
    this.animFall = Resource.getAnimation('data/images/hero_fall.bmp', 1);
    this.animHook = Resource.getAnimation('data/images/hook.bmp', 1);
    this.animHookParticle = Resource.getAnimation('data/images/particles.bmp');
    this.animHurt = Resource.getAnimation('data/images/hero_hurt.bmp', 1);
    this.animJump = Resource.getAnimation('data/images/hero_jump.bmp', 1);
    this.animRope = Resource.getAnimation('data/images/rope.bmp', 1);
    this.animRun = Resource.getAnimation('data/images/hero_run.bmp', 4);
  }

  getLayer(): number { return 2; }

  getRopePosition(): ImmutableVec2 { return this.ropePosition; }
  getRopeState(): RopeState { return this.ropeState; }

  gotCoin(): boolean {
    if (this.blinkingTicksLeft === 0) {
      GameState.put('coins', GameState.getInt('coins') + 1);
      return true;
    }
    return false;
  }

  gotCore(): boolean {
    if (this.blinkingTicksLeft === 0) {
      GameState.put('coins', GameState.getInt('coins') + 1);
      return true;
    }
    return false;
  }

  imortal(): void { this.isImmortal = true; }
  isDead(): boolean { return this.dead; }
  hasHook(): boolean { return this.ropeState === RopeState.Attached; }
  getHookedEntity(): Entity | null { return this.hookedEntity; }

  detachHook(): void {
    if (this.ropeState !== RopeState.Retracted && this.ropeState !== RopeState.Disappearing) {
      this.ropeState = RopeState.Disappearing;
      this.ropeDisappearCounter = 0;
    }
    this.hookedEntity = null;
  }

  setLastSpawnPoint(sp: ReadonlyVec2): void {
    this.spawnPoint = new ImmutableVec2(sp);
  }

  private die(): void {
    this.ropeState = RopeState.Retracted;
    this.hookedEntity = null;
    this.dead = true;
    Sound.playSample('data/sounds/start');
  }

  kill(): void {
    if (this.isImmortal) return;
    if (this.blinkingTicksLeft === 0) {
      PlayerSkill.playerDidSomethingStupid(0.0, 0.1);
      this.detachHook();

      let coins = GameState.getInt('coins');
      GameState.put('coins', 0);

      if (coins > 0) {
        coins = Math.floor(coins / 2);
        coins = Math.min(20, Math.max(0, coins));
        const lifetime = lerp(MAX_DEATH_COIN_LIFETIME, MIN_DEATH_COIN_LIFETIME, PlayerSkill.get());
        Coin.spawnDeathCoins(coins, this.getPosition(), lifetime, this.myRoom);
        this.blinkingTicksLeft = BLINKING_TICKS;
        this.myVelocity.normalize().multiply(-KILL_VELOCITY);
        Sound.playSample('data/sounds/hurt');
      } else {
        PlayerSkill.playerDidSomethingStupid(0.0, 0.25);
        this.die();
      }
    }
  }

  override remove(): void { this.die(); }

  respawn(): void {
    this.setPosition(this.spawnPoint);
    this.setVelocity(new ImmutableVec2(0, 0));
    this.dead = false;
    GameState.put('coins', 0);
    this.onGround = false;
    this.jumpHeld = false;
    this.jumpPrepressed = false;
    this.frame = 0;
    this.ropeState = RopeState.Retracted;
    this.hookedEntity = null;
    this.facingDirection = Direction.RIGHT;
    this.blinkingTicksLeft = 0;
    this.ropeDisappearCounter = 0;
  }

  getHookCollidable(): Collidable {
    const rx = this.ropePosition.x;
    const ry = this.ropePosition.y;
    return {
      getCollideLeft: () => rx - 2,
      getCollideRight: () => rx + 2,
      getCollideTop: () => ry - 2,
      getCollideBottom: () => ry + 2,
    };
  }

  private getAutoAimScore(ropeDir: ReadonlyVec2, aimPos: ReadonlyVec2): number {
    const playerToTile = new ImmutableVec2(aimPos).subtract(this.getPosition());
    let dotValue = ropeDir.dot(playerToTile);
    if (dotValue < 0) return -1;
    const distance = playerToTile.length();
    dotValue /= distance;
    if (dotValue < 0.80) return -1;
    return Math.pow(dotValue, 10.0) / distance;
  }

  private adjustRopeDirection(ropeDir: Vec2): void {
    let bestScore = 0;
    let bestDirection: ReadonlyVec2 = new ImmutableVec2(ropeDir);
    const room = this.myRoom;

    for (let y = 0; y < room.getHeightInTiles(); y++) {
      for (let x = 0; x < room.getWidthInTiles(); x++) {
        if (room.isHookable(x, y)) {
          const pixelX = x * room.getTileWidth() + room.getTileWidth() / 2;
          const pixelY = y * room.getTileHeight() + room.getTileHeight() / 2;
          const tilePos = new ImmutableVec2(pixelX, pixelY);
          const score = this.getAutoAimScore(ropeDir, tilePos);

          if (score > bestScore) {
            const direction = tilePos.subtract(this.getPosition());
            const rcX = { value: 0 };
            const rcY = { value: 0 };
            const rcHit = room.rayCast(this.getPosition(), direction, false, rcX, rcY);
            if (rcHit && rcX.value === x && rcY.value === y) {
              bestScore = score;
              bestDirection = direction;
            }
          }
        }
      }
    }

    for (const e of room.getEntities()) {
      if (!e.isDamagable() && !e.isHookable()) continue;
      const entityPos = new ImmutableVec2(e.getPosition());
      const score = this.getAutoAimScore(ropeDir, entityPos);

      if (score > bestScore) {
        const direction = entityPos.subtract(this.getPosition());
        const rcX = { value: 0 };
        const rcY = { value: 0 };
        const rcHit = room.rayCast(this.getPosition(), direction, false, rcX, rcY);
        const rcTilePos = new ImmutableVec2(
          rcX.value * room.getTileWidth() + room.getTileWidth() / 2,
          rcY.value * room.getTileHeight() + room.getTileHeight() / 2,
        );
        if (!rcHit || new Vec2(ropeDir).lengthCompare(rcTilePos) < 0) {
          bestScore = score;
          bestDirection = direction;
        }
      }
    }

    ropeDir.set(bestDirection).normalize();
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = this.getDrawPositionX();
    const y = this.getDrawPositionY();

    if (this.dead) {
      this.animHurt.drawFrame(ctx, 0,
        offsetX + x - this.animHurt.getFrameWidth() / 2,
        Math.round(offsetY + y + this.getHalfSize().y - this.animHurt.getFrameHeight()),
        this.facingDirection === Direction.LEFT, false);
      return;
    }

    let animation: Animation;
    let frame = 1;

    switch (this.movementState) {
      case MovementState.Run:
        animation = this.animRun;
        frame = Math.floor(this.frame / 10);
        break;
      case MovementState.AirRun:
        animation = this.animRun;
        frame = Math.floor(this.frame / 3);
        break;
      case MovementState.Still:
        animation = this.animRun;
        frame = 1;
        break;
      case MovementState.Jump:
        animation = this.animJump;
        frame = 1;
        break;
      case MovementState.Fall:
        animation = this.animFall;
        frame = 1;
        break;
    }

    // Draw rope
    if (this.ropeState !== RopeState.Retracted &&
        (this.ropeState !== RopeState.Disappearing || (this.ropeDisappearCounter & 1) !== 0)) {
      const x2 = this.getDrawPositionX() + offsetX;
      const y2 = this.getDrawPositionY() + offsetY;
      const x1 = Math.round(this.ropePosition.x) + offsetX;
      const y1 = Math.round(this.ropePosition.y) + offsetY;

      const lineVec = new Vec2(x2 - x1, y2 - y1);
      const rlength = Math.round(lineVec.length());
      lineVec.normalize().multiply(3);

      const segments = Math.ceil(rlength / lineVec.length());
      let rx = x1;
      let ry = y1;

      for (let i = 0; i < segments; i++) {
        let waveX = 0;
        let waveY = 0;

        if ((this.ropeState === RopeState.Moving || this.ropeState === RopeState.Disappearing) && segments > 1) {
          const t = i / (segments - 1);
          const value = Math.sin(t * Math.PI) * Math.sin(i * 0.3);
          waveX = lineVec.y * value;
          waveY = -lineVec.x * value;
        }

        this.animRope.drawFrame(ctx, frame,
          Math.round(rx + waveX - this.animRope.getFrameWidth() / 2),
          Math.round(ry + waveY - this.animRope.getFrameHeight() / 2));

        rx += lineVec.x;
        ry += lineVec.y;
      }

      this.animHook.drawFrame(ctx, frame,
        x1 - this.animHook.getFrameWidth() / 2,
        y1 - this.animHook.getFrameHeight() / 2);
    }

    if (this.blinkingTicksLeft > 0 && !this.onGround) {
      animation = this.animHurt;
    }

    if (Math.floor(this.blinkingTicksLeft / 6) % 2 === 0) {
      animation.drawFrame(ctx, frame,
        offsetX + x - animation.getFrameWidth() / 2,
        Math.round(offsetY + y + this.getHalfSize().y - animation.getFrameHeight()),
        this.facingDirection === Direction.LEFT, false);
    }
  }

  override update(): void {
    if (this.dead) return;
    super.update();

    if (this.spawnPoint.x < -100 && this.spawnPoint.y < -100) {
      this.spawnPoint = new ImmutableVec2(this.getPosition());
    }

    if (this.ropeState === RopeState.Disappearing) {
      this.ropeDisappearCounter++;
      if (this.ropeDisappearCounter >= ROPE_DISAPPEAR_TICKS) {
        this.ropeState = RopeState.Retracted;
      }
    }

    const acceleration = this.onGround ? GROUND_ACCELERATION : AIR_ACCELERATION;
    let airRunning = false;

    if (Input.isHeld(Button.LEFT)) {
      this.myVelocity.subtract(acceleration, 0);
      if (this.ropeState === RopeState.Attached && !this.onGround) {
        this.facingDirection = Direction.LEFT;
        airRunning = true;
        this.movementState = MovementState.AirRun;
      }
    }

    if (Input.isHeld(Button.RIGHT)) {
      this.myVelocity.add(acceleration, 0);
      if (this.ropeState === RopeState.Attached && !this.onGround) {
        this.facingDirection = Direction.RIGHT;
        airRunning = true;
        this.movementState = MovementState.AirRun;
      }
    }

    if (Input.isPressed(Button.JUMP)) {
      if (!this.jumpPrepressed && this.onGround) Sound.playSample('data/sounds/jump');
      this.jumpPrepressed = true;
    }

    if (this.onGround && this.jumpPrepressed) {
      this.myVelocity.subtract(0, JUMP_VELOCITY);
      if (this.ropeState !== RopeState.Attached) {
        this.jumpHeld = true;
      }
      this.jumpPrepressed = false;
    }

    if (Input.isReleased(Button.JUMP)) {
      if (this.jumpHeld && this.myVelocity.y < 0) {
        this.myVelocity.set(this.myVelocity.x, this.myVelocity.y * 0.5);
      }
      this.jumpHeld = false;
      this.jumpPrepressed = false;
    }

    if (Input.isReleased(Button.FIRE)) {
      this.detachHook();
    }

    if (this.myVelocity.y >= 0) {
      this.jumpHeld = false;
    }

    if (!airRunning) {
      this.movementState = MovementState.Still;
    }

    if (this.onGround) {
      if (this.myVelocity.x > 0) this.facingDirection = Direction.RIGHT;
      else if (this.myVelocity.x < 0) this.facingDirection = Direction.LEFT;
      if (Math.abs(this.myVelocity.x) > GROUND_STOP_VELOCITY) {
        this.movementState = MovementState.Run;
      }
    } else if (!airRunning) {
      this.movementState = this.myVelocity.y > 0 ? MovementState.Jump : MovementState.Fall;
    }

    if (this.movementState === MovementState.Still) {
      this.myVelocity.set(0, this.myVelocity.y);
    }

    // Fire hook
    if (Input.isPressed(Button.FIRE)) {
      Sound.playSample('data/sounds/rope');
      this.ropeState = RopeState.Moving;
      this.ropePosition = new ImmutableVec2(this.getPosition());
      this.ropeVelocity.set(0, 0);

      if (Input.isHeld(Button.LEFT)) this.ropeVelocity.subtract(1, 0);
      if (Input.isHeld(Button.RIGHT)) this.ropeVelocity.add(1, 0);
      if (Input.isHeld(Button.UP)) this.ropeVelocity.subtract(0, 1);
      if (Input.isHeld(Button.DOWN)) this.ropeVelocity.add(0, 1);

      if (this.ropeVelocity.isZero()) {
        this.ropeVelocity.set(this.facingDirection === Direction.LEFT ? -1 : 1, this.ropeVelocity.y);
      }

      this.ropeVelocity.normalize().multiply(ROPE_SPEED).add(this.myVelocity).normalize();
      this.adjustRopeDirection(this.ropeVelocity);
      this.ropeVelocity.multiply(ROPE_SPEED);

      if (this.ropeVelocity.x < 0) this.facingDirection = Direction.LEFT;
      else if (this.ropeVelocity.x > 0) this.facingDirection = Direction.RIGHT;
    }

    // Move rope
    if (this.ropeState === RopeState.Moving) {
      const substeps = 25;
      for (let i = 0; i < substeps; i++) {
        this.ropePosition = this.ropePosition.add(
          new ImmutableVec2(this.ropeVelocity).divide(substeps * TICKS_PER_SECOND));
        const tileX = Math.floor(this.ropePosition.x / this.myRoom.getTileWidth());
        const tileY = Math.floor(this.ropePosition.y / this.myRoom.getTileHeight());

        if (this.myRoom.isHookable(tileX, tileY)) {
          Sound.playSample('data/sounds/hook');
          this.ropeState = RopeState.Attached;
          this.jumpHeld = false;

          const ps = new ParticleSystem(
            this.animHookParticle, 2, 30, 10, 1, 50, 10,
            new Vec2(this.ropeVelocity).normalize().multiply(-10),
            1.0);
          ps.setPosition(this.ropePosition);
          this.myRoom.addEntity(ps);
          break;
        }

        if (this.ropePosition.subtract(this.getPosition()).length() > this.ropeMaxLength) {
          this.detachHook();
          Sound.playSample('data/sounds/no_hook');
          break;
        }

        if (this.myRoom.isCollidable(tileX, tileY)) {
          this.detachHook();
          Sound.playSample('data/sounds/no_hook');
          break;
        }

        if (this.myRoom.damageDone(Math.floor(this.ropePosition.x), Math.floor(this.ropePosition.y))) {
          this.detachHook();
          break;
        }

        this.hookedEntity = this.myRoom.findHookableEntity(this.ropePosition);
        if (this.hookedEntity !== null) {
          Sound.playSample('data/sounds/hook');
          this.ropeState = RopeState.Attached;
          this.jumpHeld = false;
          const off = this.ropePosition.subtract(this.hookedEntity.getPosition());
          this.hookedEntityOffset = new ImmutableVec2(Math.floor(off.x), Math.floor(off.y));
        }
      }
    }

    // Attached rope physics
    if (this.ropeState === RopeState.Attached) {
      if (this.hookedEntity !== null) {
        this.ropePosition = new ImmutableVec2(this.hookedEntity.getPosition()).add(this.hookedEntityOffset);
      }

      const ropeToHero = new ImmutableVec2(this.getPosition()).subtract(this.ropePosition);
      if (ropeToHero.lengthCompare(ROPE_REST_LENGTH) > 0) {
        const ropeRestPoint = this.ropePosition.add(ropeToHero.normalize().multiply(ROPE_REST_LENGTH));
        const heroToRestPoint = ropeRestPoint.subtract(this.getPosition());
        let ropeAcceleration = heroToRestPoint.multiply(ROPE_SPRING_CONSTANT);
        if (ropeAcceleration.lengthCompare(ROPE_MAX_ACCELERATION) > 0) {
          ropeAcceleration = ropeAcceleration.normalize().multiply(ROPE_MAX_ACCELERATION);
        }
        this.myVelocity.add(ropeAcceleration);
      }

      if (Input.isHeld(Button.UP)) this.myVelocity.subtract(0, acceleration);
      if (Input.isHeld(Button.DOWN)) this.myVelocity.add(0, acceleration);

      const ropeTileX = Math.floor(this.ropePosition.x / this.myRoom.getTileWidth());
      const ropeTileY = Math.floor(this.ropePosition.y / this.myRoom.getTileWidth());
      if (this.hookedEntity === null && !this.myRoom.isHookable(ropeTileX, ropeTileY)) {
        this.detachHook();
      }
    }

    const bumps = this.moveWithCollision();

    if (bumps.has(Direction.LEFT) || bumps.has(Direction.RIGHT)) {
      this.myVelocity.set(0, this.myVelocity.y);
    }
    if (bumps.has(Direction.UP) || bumps.has(Direction.DOWN)) {
      this.myVelocity.set(this.myVelocity.x, 0);
    }

    const gravity = this.jumpHeld ? JUMP_GRAVITY : GRAVITY;
    this.myVelocity.add(0, gravity);
    const ground = bumps.has(Direction.DOWN);
    if (ground && !this.onGround && this.ropeState !== RopeState.Attached) {
      Sound.playSample('data/sounds/land');
    }
    this.onGround = ground;

    const drag = this.onGround ? GROUND_DRAG : AIR_DRAG;
    this.myVelocity.multiply(drag);

    this.frame++;

    if (this.blinkingTicksLeft > 0) {
      this.blinkingTicksLeft--;
    }
  }
}
