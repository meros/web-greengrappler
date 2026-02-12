import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import type { Animation } from '../media/animation.js';
import type { Room } from '../room.js';
import { ParticleSystem } from './particleSystem.js';
import { ReactorCore } from './reactorCore.js';

const BLOW_TIME = 60 * 4 + 30;
const DAMAGE_MAX = 16;
const FRAME_PER_DAMAGE = 4;

export class Reactor extends Entity {
  private aboutToBlow = false;
  private damage = 0;
  private frameCounter = 0;
  private readonly shell: Animation;

  constructor() {
    super();
    this.setSize(new ImmutableVec2(30, 40));
    this.shell = Resource.getAnimation('data/images/reactor_shell.bmp', 4);
  }

  getLayer(): number { return 1; }
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
    if (this.damage >= DAMAGE_MAX) return;

    const numPs = Math.abs(Math.floor(Math.random() * 5)) % 5 + 1;
    const ps = new ParticleSystem(
      Resource.getAnimation('data/images/debris.bmp', 4),
      10, 40, 10, 1, 50, numPs, new ImmutableVec2(0, -20), 2.0);
    ps.setPositionWithSpread(this.getPosition(), 10, false);
    this.myRoom.addEntity(ps);

    this.damage++;
    if (this.damage === DAMAGE_MAX) {
      this.frameCounter = 1;
      this.aboutToBlow = true;
      Sound.playSample('data/sounds/damage');
      this.myRoom.getCamera().addShake(4.0, BLOW_TIME);
    } else {
      Sound.playSample('data/sounds/damage');
      this.myRoom.getCamera().addShake(1.0, 20);
    }
  }

  override onRespawn(): void {
    this.aboutToBlow = false;
    this.damage = 0;
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = this.getDrawPositionX() + offsetX;
    const y = this.getDrawPositionY() + offsetY;
    const frame = this.damage >= DAMAGE_MAX
      ? Math.floor(this.damage / FRAME_PER_DAMAGE) - 1
      : Math.floor(this.damage / FRAME_PER_DAMAGE);
    this.shell.drawFrame(ctx, frame,
      x - Math.floor(this.getSize().x / 2),
      y - Math.floor(this.getSize().y / 2));
  }

  override update(): void {
    this.frameCounter++;

    if (this.aboutToBlow && this.frameCounter % 60 === 0)
      Sound.playSample('data/sounds/alarm');

    if (this.aboutToBlow && this.frameCounter % 20 === 0) {
      const numPs = Math.abs(Math.floor(Math.random() * 5)) % 5 + 1;
      const ps = new ParticleSystem(
        Resource.getAnimation('data/images/debris.bmp', 4),
        10, 40, 10, 1, 50, numPs, new ImmutableVec2(0, -20), 2.0);
      ps.setPositionWithSpread(this.getPosition(), 10, false);
      this.myRoom.addEntity(ps);
    }

    if (this.aboutToBlow && this.frameCounter >= BLOW_TIME) {
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

      this.setTilesCollidable(false);
      this.remove();
    }
  }
}
