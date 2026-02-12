import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { Resource } from '../resource.js';
import type { Animation } from '../media/animation.js';

export class Door extends Entity {
  private readonly id: number;
  private opening = false;
  private closing = false;
  private doorHeight = 40;
  private frameCounter = 0;
  private readonly doorAnim: Animation;

  constructor(id: number) {
    super();
    this.id = id;
    this.setSize(new ImmutableVec2(10, 40));
    this.doorAnim = Resource.getAnimation('data/images/door.bmp', 1);
  }

  getLayer(): number { return 0; }

  override onRespawn(): void {
    this.opening = false;
    this.closing = false;
    this.doorHeight = 40;
  }

  override onButtonDown(id: number): void {
    if (this.id !== id) return;
    this.frameCounter = 0;
    this.opening = true;
    this.closing = false;
  }

  override onButtonUp(id: number): void {
    if (this.id !== id) return;
    this.closing = true;
    this.opening = false;
    this.frameCounter = 0;
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = Math.round(this.getDrawPositionX() + offsetX - this.getSize().x / 2);
    const y = Math.round(this.getDrawPositionY() + offsetY - this.getSize().y / 2);

    // Draw partial door using drawScaled to crop
    const img = this.doorAnim.getGameImage();
    if (img) {
      const sub = img.subImage(0, 0, 10, this.doorHeight);
      sub.draw(ctx, x, y);
    }
  }

  override update(): void {
    this.frameCounter++;

    if (this.doorHeight < 4) this.doorHeight = 4;

    if (this.opening && this.doorHeight !== 4) {
      this.doorHeight -= 2;
    } else if (this.opening) {
      this.frameCounter = 0;
    }

    if (this.closing && this.frameCounter % 5 === 0 && this.doorHeight !== 40) {
      this.doorHeight++;
    }

    const tileX = Math.floor((this.getPosition().x - this.getSize().x / 2) / 10);
    const tileY = Math.floor((this.getPosition().y - this.getSize().y / 2) / 10);

    for (let i = 0; i < 4; i++) {
      this.myRoom.setCollidable(tileX, tileY + i, i <= Math.floor(this.doorHeight / 10));
    }
  }
}
