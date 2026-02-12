import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';

export class BossFloor extends Entity {
  private active = false;
  private frameCounter = 0;

  constructor() {
    super();
    this.setSize(new ImmutableVec2(320, 10));
  }

  getLayer(): number { return 4; }

  override getCollideTop(): number { return super.getCollideTop() - 1; }

  override onBossFloorActivate(): void {
    this.active = true;
    this.frameCounter = 0;
  }

  override onRespawn(): void { this.active = false; }

  override draw(_ctx: CanvasRenderingContext2D, _ox: number, _oy: number, _layer: number): void {}

  override update(): void {
    this.frameCounter++;
    if (!this.active) return;
    if (this.myRoom.getHero().Collides(this)) this.myRoom.getHero().kill();
    if (this.frameCounter > 60) this.active = false;
  }
}
