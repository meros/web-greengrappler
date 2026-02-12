import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { Resource } from '../resource.js';
import type { Animation } from '../media/animation.js';
import { ParticleSystem } from './particleSystem.js';

export class LavaSea extends Entity {
  private frame = 0;
  private safeMode = false;
  private safeLevel = 0;
  private readonly topAnim: Animation;
  private readonly fillAnim: Animation;

  constructor() {
    super();
    this.topAnim = Resource.getAnimation('data/images/lavatop.bmp', 2);
    this.fillAnim = Resource.getAnimation('data/images/lavafill.bmp', 2);
    this.setSize(new ImmutableVec2(10, 10));
  }

  getLayer(): number { return 4; }

  override getCollideTop(): number { return this.getCurrentY(); }
  override getCollideLeft(): number { return 0; }
  override getCollideRight(): number { return 100000; }
  override getCollideBottom(): number { return 100000; }

  private getCurrentY(): number {
    if (this.safeMode) return this.safeLevel;
    let sinwave = (Math.sin(this.frame / 100.0) + 1) / 2;
    sinwave = sinwave * sinwave * sinwave;
    return Math.round(this.getPosition().y - sinwave * 80 + 2);
  }

  override onLevelComplete(): void {
    this.safeLevel = this.getCurrentY();
    this.safeMode = true;
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const screenY = offsetY + this.getCurrentY();
    for (let x = -10 + offsetX % 10; x < 320; x += 10) {
      this.topAnim.drawFrame(ctx, Math.floor(this.frame / 30), x, screenY);
    }
    for (let x = -10 + offsetX % 10; x < 320; x += 10) {
      for (let y = screenY + 10; y < 240; y += 10) {
        this.fillAnim.drawFrame(ctx, Math.floor(this.frame / 30), x, y);
      }
    }
  }

  override update(): void {
    if (this.myRoom.getHero().Collides(this)) {
      this.myRoom.getHero().kill();
    }

    this.frame++;

    if (this.safeMode) {
      this.safeLevel++;
      if (this.safeLevel > this.getPosition().y + 10) {
        this.safeLevel = Math.round(this.getPosition().y + 10);
      }
    }

    if (Math.random() < 0.3) {
      const ps = new ParticleSystem(
        Resource.getAnimation('data/images/particles.bmp'),
        2, 30, 10, 1, 50, 10, new ImmutableVec2(0, -30), 5.0);
      const min = -this.myRoom.getCamera().getOffsetX();
      const max = 320 - this.myRoom.getCamera().getOffsetX();
      const rVal = Math.floor(Math.random() * (max - min)) + min;
      ps.setPositionWithSpread(new ImmutableVec2(rVal, this.getCurrentY()), 5, false);
      this.myRoom.addEntity(ps);
    }
  }
}
