import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import type { Animation } from '../media/animation.js';

export class Button extends Entity {
  private triggered = false;
  private readonly time = 60 * 5;
  private counter: number;
  private readonly id: number;
  private readonly buttonAnim: Animation;

  constructor(id: number) {
    super();
    this.id = id;
    this.counter = this.time + 1;
    this.setSize(new ImmutableVec2(10, 10));
    this.buttonAnim = Resource.getAnimation('data/images/button.bmp');
  }

  getLayer(): number { return 0; }

  override onRespawn(): void {
    this.triggered = false;
    this.counter = this.time + 1;
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = this.getDrawPositionX() + offsetX;
    const y = this.getDrawPositionY() + offsetY;
    const frame = this.counter > this.time ? 0 : 1;
    this.buttonAnim.drawFrame(ctx, frame,
      x - Math.floor(this.getSize().x / 2),
      y - Math.floor(this.getSize().y / 2));
  }

  override update(): void {
    this.counter++;

    if (this.counter === this.time) {
      Sound.playSample('data/sounds/timeout');
      this.myRoom.broadcastButtonUp(this.id);
      return;
    }

    if (this.counter < this.time) {
      if (this.counter % 60 === 0) Sound.playSample('data/sounds/time');
      return;
    }

    if (this.myRoom.getHero().Collides(this)) {
      if (!this.triggered) {
        this.triggered = true;
        this.counter = 0;
        Sound.playSample('data/sounds/time');
        this.myRoom.broadcastButtonDown(this.id);
      }
    } else {
      this.triggered = false;
    }
  }
}
