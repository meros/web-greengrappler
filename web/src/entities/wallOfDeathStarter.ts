import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';

export class WallOfDeathStarter extends Entity {
  private used = false;

  constructor() {
    super();
    this.setSize(new ImmutableVec2(10, 10 * 100));
  }

  getLayer(): number { return 1; }

  override onRespawn(): void { this.used = false; }

  override draw(_ctx: CanvasRenderingContext2D, _ox: number, _oy: number, _layer: number): void {}

  override update(): void {
    if (!this.used && this.myRoom.getHero().Collides(this)) {
      this.myRoom.broadcastStartWallOfDeath();
      this.used = true;
    }
  }
}
