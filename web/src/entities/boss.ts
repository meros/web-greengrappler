import { Direction } from '../constants.js';
import { ImmutableVec2 } from '../math.js';
import { lerp } from '../utils.js';
import { Entity } from '../entity.js';
import { PlayerSkill } from '../playerSkill.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import { Music } from '../media/music.js';
import type { Animation } from '../media/animation.js';
import type { Room } from '../room.js';
import { ParticleSystem } from './particleSystem.js';
import { ReactorCore } from './reactorCore.js';
import { Coin } from './coin.js';

const BLOW_TIME = 60 * 4 + 30;
const DAMAGE_MAX = 16;
const FRAME_PER_DAMAGE = 4;
const INITIAL_BLOW_TIME = 60 * 4 + 30;
const MAX_HEALTH = 40;
const MIN_HEALTH = 10;

const enum BossState {
  INIT, SLEEPING, INITIAL_BLOW, AWAKENING, MOVE_UPWARDS, FLOAT, STOP, ATTACK, VULNERABLE, DEAD
}

export class Boss extends Entity {
  private health = lerp(MAX_HEALTH, MIN_HEALTH, PlayerSkill.get());
  private state: BossState = BossState.INIT;
  private direction: Direction = Direction.LEFT;
  private initialHealth = 16;
  private animFrameCounter = 0;
  private readonly reactorAnim: Animation;
  private readonly bossAnim: Animation;
  private frameCounter = 0;
  private originalPos = new ImmutableVec2();

  constructor() {
    super();
    this.setSize(new ImmutableVec2(30, 40));
    this.reactorAnim = Resource.getAnimation('data/images/reactor_shell.bmp', 4);
    this.bossAnim = Resource.getAnimation('data/images/boss.bmp', 4);
  }

  getLayer(): number { return 3; }
  override isDamagable(): boolean { return true; }

  override getCollideTop(): number { return this.getPosition().y - this.getHalfSize().y - 2; }
  override getCollideLeft(): number { return this.getPosition().x - this.getHalfSize().x - 2; }
  override getCollideBottom(): number { return this.getPosition().y + this.getHalfSize().y + 2; }
  override getCollideRight(): number { return this.getPosition().x + this.getHalfSize().x + 2; }

  override setRoom(room: Room): void {
    super.setRoom(room);
    this.setTilesCollidable(true);
  }

  private setTilesCollidable(v: boolean): void {
    const sx = Math.floor((this.getPosition().x - this.getHalfSize().x) / 10);
    const sy = Math.floor((this.getPosition().y - this.getHalfSize().y) / 10);
    for (let x = sx; x < sx + 3; x++)
      for (let y = sy; y < sy + 4; y++)
        this.myRoom.setCollidable(x, y, v);
  }

  override onDamage(): void {
    if (this.state === BossState.SLEEPING) {
      this.spawnDebris();
      this.initialHealth--;
      if (this.initialHealth === 0) {
        this.frameCounter = 0;
        this.state = BossState.INITIAL_BLOW;
        Sound.playSample('data/sounds/damage');
        this.myRoom.getCamera().addShake(4.0, INITIAL_BLOW_TIME);
      } else {
        Sound.playSample('data/sounds/damage');
        this.myRoom.getCamera().addShake(1.0, 20);
      }
    }

    if (this.state === BossState.VULNERABLE) {
      this.spawnDebris();
      this.health--;
      if (this.health < 0) {
        this.state = BossState.DEAD;
        this.frameCounter = 0;
        Sound.playSample('data/sounds/damage');
        this.myRoom.getCamera().addShake(4.0, BLOW_TIME);
      } else {
        Sound.playSample('data/sounds/damage');
        this.myRoom.getCamera().addShake(1.0, 20);
      }
    }
  }

  private spawnDebris(): void {
    const numPs = Math.abs(Math.floor(Math.random() * 5)) % 5 + 1;
    const ps = new ParticleSystem(
      Resource.getAnimation('data/images/debris.bmp', 4),
      10, 40, 10, 1, 50, numPs, new ImmutableVec2(0, -20), 2.0);
    ps.setPositionWithSpread(this.getPosition(), 10, false);
    this.myRoom.addEntity(ps);
  }

  override onRespawn(): void {
    this.setPosition(this.originalPos);
    this.state = BossState.SLEEPING;
    this.direction = Direction.LEFT;
    this.health = lerp(MAX_HEALTH, MIN_HEALTH, PlayerSkill.get());
    this.initialHealth = 16;
    this.animFrameCounter = 0;
    Music.popSong();
    this.setTilesCollidable(true);
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = Math.round(this.getDrawPositionX() + offsetX - this.getHalfSize().x);
    const y = Math.round(this.getDrawPositionY() + offsetY - this.getHalfSize().y);

    if (this.state === BossState.SLEEPING)
      this.reactorAnim.drawFrame(ctx,
        Math.floor(DAMAGE_MAX / FRAME_PER_DAMAGE) - Math.floor((this.initialHealth + 3) / FRAME_PER_DAMAGE), x, y);
    else if (this.state === BossState.INITIAL_BLOW)
      this.reactorAnim.drawFrame(ctx, 3, x, y);
    else if (this.state === BossState.VULNERABLE)
      this.bossAnim.drawFrame(ctx, 0, x, y);
    else
      this.bossAnim.drawFrame(ctx, Math.floor(this.animFrameCounter / 5), x, y);
  }

  override update(): void {
    this.animFrameCounter++;
    this.frameCounter++;

    if (this.state === BossState.INIT) {
      this.originalPos = new ImmutableVec2(this.getPosition());
      this.state = BossState.SLEEPING;
    }

    if (this.state !== BossState.MOVE_UPWARDS && this.state !== BossState.SLEEPING &&
        this.state !== BossState.INITIAL_BLOW && this.state !== BossState.DEAD &&
        this.state !== BossState.VULNERABLE) {
      if (this.myRoom.getHero().Collides(this)) this.myRoom.getHero().kill();
    }

    if (this.state === BossState.INITIAL_BLOW) {
      if (this.frameCounter % 60 === 0) Sound.playSample('data/sounds/alarm');
      if (this.frameCounter >= INITIAL_BLOW_TIME) {
        Sound.playSample('data/sounds/reactor_explosion');
        Sound.playSample('data/sounds/start');
        const ps = new ParticleSystem(
          Resource.getAnimation('data/images/debris.bmp', 4),
          20, 200, 20, 1, 50, 50, new ImmutableVec2(0, -150), 5.0);
        ps.setPositionWithSpread(this.getPosition(), 10, false);
        this.myRoom.addEntity(ps);
        this.frameCounter = 0;
        this.state = BossState.AWAKENING;
      }
    }

    if (this.state === BossState.AWAKENING) {
      if (this.frameCounter > 60 * 3) {
        this.state = BossState.MOVE_UPWARDS;
        this.frameCounter = 0;
        this.setTilesCollidable(false);
        Music.pushSong();
        Music.playSong('data/music/olof3.xm');
      }
    }

    if (this.state === BossState.MOVE_UPWARDS) {
      this.setPosition(new ImmutableVec2(this.getPosition().x, this.getPosition().y - 1));
      if (this.originalPos.y - this.getPosition().y > 130) {
        this.state = BossState.FLOAT;
        this.myRoom.broadcastBoosWallActivate();
        this.frameCounter = 0;
      }
    }

    if (this.state === BossState.FLOAT) {
      const yDelta = Math.sin(this.frameCounter / 8) * 2;
      if (this.direction === Direction.RIGHT) {
        this.setPosition(new ImmutableVec2(this.getPosition().x + 1.5, this.getPosition().y + yDelta));
        if (this.getPosition().x - this.originalPos.x > 110) this.direction = Direction.LEFT;
      }
      if (this.direction === Direction.LEFT) {
        this.setPosition(new ImmutableVec2(this.getPosition().x - 1.5, this.getPosition().y + yDelta));
        if (this.originalPos.x - this.getPosition().x > 110) this.direction = Direction.RIGHT;
      }
      if (this.frameCounter > 60 * 7) {
        this.state = BossState.ATTACK;
        this.frameCounter = 0;
      }
    }

    if (this.state === BossState.ATTACK) {
      if (this.frameCounter < 40) {
        if (this.frameCounter % 2 === 0)
          this.setPosition(new ImmutableVec2(this.getPosition().x - 2, this.getPosition().y));
        else
          this.setPosition(new ImmutableVec2(this.getPosition().x + 2, this.getPosition().y));
      } else {
        this.setPosition(new ImmutableVec2(this.getPosition().x, this.getPosition().y + 6));
        if (this.originalPos.y < this.getPosition().y) {
          this.myRoom.broadcastBoosWallDectivate();
          this.setPosition(new ImmutableVec2(this.getPosition().x, this.originalPos.y));
          this.state = BossState.VULNERABLE;
          this.myRoom.getCamera().addShake(5.0, 80);

          const numPs = Math.abs(Math.floor(Math.random() * 10)) % 10 + 5;
          const ps = new ParticleSystem(
            Resource.getAnimation('data/images/debris.bmp', 4),
            10, 40, 10, 1, 50, numPs, new ImmutableVec2(0, -20), 2.0);
          ps.setPositionWithSpread(this.getPosition(), 10, false);
          this.myRoom.addEntity(ps);
          this.myRoom.broadcastBoosFloorActivate();
          Sound.playSample('data/sounds/reactor_explosion');
          Coin.spawnDeathCoins(
            Math.abs(Math.floor(Math.random() * 5)) % 5 + 3,
            this.getPosition(), 60 * 6, this.myRoom);
        }
      }
    }

    if (this.state === BossState.VULNERABLE) {
      if (this.frameCounter > 60 * 5) {
        this.state = BossState.MOVE_UPWARDS;
        this.frameCounter = 0;
      }
    }

    if (this.state === BossState.DEAD) {
      if (this.frameCounter % 60 === 0) Sound.playSample('data/sounds/alarm');
      if (this.frameCounter >= BLOW_TIME) {
        Sound.playSample('data/sounds/reactor_explosion');
        Sound.playSample('data/sounds/start');
        const ps = new ParticleSystem(
          Resource.getAnimation('data/images/debris.bmp', 4),
          20, 200, 20, 1, 50, 50, new ImmutableVec2(0, -150), 5.0);
        ps.setPositionWithSpread(this.getPosition(), 10, false);
        this.myRoom.addEntity(ps);
        const core = new ReactorCore();
        core.setPosition(this.getPosition());
        core.setVelocity(new ImmutableVec2(0, -50));
        this.myRoom.addEntity(core);
        this.remove();
      }
    }
  }
}
