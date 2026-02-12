import { Vec2, ImmutableVec2, type ReadonlyVec2 } from './math.js';
import { GlobalOptions, VibrationType } from './options.js';
import type { Hero } from './entities/hero.js';

interface CameraRect {
  readonly x: number;
  readonly y: number;
  readonly w: number;
  readonly h: number;
}

export class Camera {
  private readonly offset = new Vec2();
  private readonly shakeOffset = new Vec2();
  private shakeAmount = 0;
  private shakeTime = 0;
  private readonly topLeft: ImmutableVec2;
  private readonly bottomRight: ImmutableVec2;
  private readonly rects: CameraRect[] = [];
  private readonly desiredOffset = new Vec2();

  constructor(topLeft: ImmutableVec2, bottomRight: ImmutableVec2) {
    this.topLeft = topLeft;
    this.bottomRight = bottomRight;
  }

  addRect(x: number, y: number, w: number, h: number): void {
    this.rects.push({ x, y, w, h });
  }

  addShake(amount: number, time: number): void {
    this.shakeAmount = amount;
    this.shakeTime = time;
    GlobalOptions.vibrate(Math.floor(time / 60 * 1000), VibrationType.PULSATING);
  }

  centerToHero(hero: Hero): void {
    this.offset.set(
      -hero.getDrawPositionX() + 160,
      -hero.getDrawPositionY() + Math.floor((2 * 240) / 3),
    );
  }

  getOffsetX(): number { return Math.round(this.offset.x + this.shakeOffset.x); }
  getOffsetY(): number { return Math.round(this.offset.y + this.shakeOffset.y); }

  update(hero: Hero): void {
    let foundRect = false;
    const heroPos = hero.getPosition();

    for (const rect of this.rects) {
      if (rect.x < heroPos.x && rect.y < heroPos.y &&
          rect.x + rect.w > heroPos.x && rect.y + rect.h > heroPos.y) {
        this.desiredOffset.set(-rect.x, -rect.y + 10);
        foundRect = true;
      }
    }

    if (!foundRect) {
      this.desiredOffset.set(
        -hero.getDrawPositionX() + 160,
        -hero.getDrawPositionY() + Math.floor((2 * 240) / 3),
      );
    }

    this.desiredOffset.subtract(this.offset).multiply(0.1);
    this.offset.add(this.desiredOffset);

    this.shakeTime--;
    if (this.shakeTime > 0) {
      this.shakeOffset.set(
        this.shakeAmount * (Math.random() - 0.5),
        this.shakeAmount * (Math.random() - 0.5),
      );
    }
    if (this.shakeTime < 0) {
      this.shakeOffset.set(0, 0);
    }

    if (this.offset.x + this.topLeft.x > 0)
      this.offset.x = -this.topLeft.x;
    if (this.offset.y + this.topLeft.y > 10)
      this.offset.y = 10 - this.topLeft.y;
    if (this.offset.x + this.bottomRight.x < 320)
      this.offset.x = 320 - this.bottomRight.x;
    if (this.offset.y + this.bottomRight.y < 240)
      this.offset.y = 240 - this.bottomRight.y;
  }

  onRespawn(): void {
    this.shakeTime = 0;
    this.shakeAmount = 0;
  }
}
