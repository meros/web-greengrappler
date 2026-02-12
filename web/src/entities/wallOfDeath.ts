import { ImmutableVec2 } from '../math.js';
import { lerp } from '../utils.js';
import { Entity } from '../entity.js';
import { PlayerSkill } from '../playerSkill.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import type { Animation } from '../media/animation.js';
import { ParticleSystem } from './particleSystem.js';

export class WallOfDeath extends Entity {
  private inited = false;
  private boost = false;
  private speed = 0;
  private frameCounter = 0;
  private running = false;
  private soundCountdown = 10;
  private readonly saw: Animation;
  private originalPosition = new ImmutableVec2();

  constructor() {
    super();
    this.setSize(new ImmutableVec2(10, 12 * 100));
    this.saw = Resource.getAnimation('data/images/saw.bmp');
  }

  getLayer(): number { return 1; }

  override onRespawn(): void {
    this.myRoom.destroyToTileRow(-1);
    this.setPosition(this.originalPosition);
    this.running = false;
    this.boost = false;
    this.soundCountdown = 10;
  }

  override onStartWallOfDeath(): void {
    if (!this.running) {
      this.running = true;
    } else {
      this.boost = true;
    }
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = Math.round(this.getDrawPositionX() + offsetX - this.getSize().x / 2);
    const y = Math.round(this.getDrawPositionY() + offsetY - this.getSize().y / 2);
    const saws = Math.floor(this.getSize().y / 12);

    for (let i = 0; i < saws; i++) {
      this.saw.drawFrame(ctx, Math.floor(this.frameCounter / 5) + i, x, y + i * 12);
    }
  }

  override update(): void {
    this.frameCounter++;

    if (!this.inited) {
      this.originalPosition = new ImmutableVec2(this.getPosition());
      this.inited = true;
    }

    if (!this.running) return;

    if (this.getPosition().x > 2950) {
      const ps = new ParticleSystem(this.saw, 2, 100, 20, 10, 20, 1, new ImmutableVec2(0, -50), 4.0);
      const x = Math.round(this.getDrawPositionX() - this.getSize().x / 2);
      const y = Math.round(this.getDrawPositionY() - this.getSize().y / 2);
      const saws = Math.floor(this.getSize().y / 12);

      for (let i = 0; i < saws; i++) {
        ps.setPositionWithSpread(new ImmutableVec2(x, y + i * 12), 0.1, true);
      }
      this.myRoom.addEntity(ps);
      this.remove();
      return;
    }

    for (let xo = 0; xo < 3; xo++) {
      const xt = xo + Math.floor((this.getPosition().x - this.getSize().x / 2) / this.myRoom.getTileWidth());
      this.myRoom.destroyToTileRow(xt - 1);
    }

    const heroX = this.myRoom.getHero().getPosition().x;
    const x = this.getPosition().x;
    const distance = heroX - x;

    if (distance > 190 && !this.boost) {
      this.setPosition(new ImmutableVec2(heroX - 190, this.getPosition().y));
    }

    if (distance < 80) this.boost = false;

    if (this.boost) {
      this.speed = lerp(5, 9, PlayerSkill.get());
    } else {
      const wantedSpeed = lerp(1, 1.9, PlayerSkill.get());
      this.speed = lerp(this.speed, wantedSpeed, 0.05);
    }

    this.soundCountdown -= this.speed;
    if (this.soundCountdown <= 0) {
      this.soundCountdown += 10;
      Sound.playSample('data/sounds/damage');
    }

    this.setPosition(new ImmutableVec2(this.getPosition().x + this.speed, this.getPosition().y));

    if (this.myRoom.getHero().Collides(this)) {
      this.myRoom.getHero().kill();
    }
  }
}
