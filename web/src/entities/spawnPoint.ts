import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { PlayerSkill } from '../playerSkill.js';
import { Resource } from '../resource.js';
import type { Animation } from '../media/animation.js';

const enum SpawnState { UNCHECKED = 0, SEMI_CHECKED = 1, CHECKED = 2 }

export class SpawnPoint extends Entity {
  private state: SpawnState = SpawnState.UNCHECKED;
  private frame = 0;
  private readonly animation: Animation;

  constructor() {
    super();
    this.setSize(new ImmutableVec2(22, 25));
    this.animation = Resource.getAnimation('data/images/checkpoint.bmp', 3);
  }

  getLayer(): number { return 3; }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    const x = Math.round(this.getPosition().x - this.animation.getFrameWidth() / 2 + offsetX);
    const y = Math.round(this.getPosition().y - this.animation.getFrameHeight() / 2 + offsetY);
    this.animation.drawFrame(ctx, this.state, x, y);
  }

  override update(): void {
    const hero = this.myRoom.getHero();
    if (hero.Collides(this)) {
      if (this.state === SpawnState.UNCHECKED) {
        PlayerSkill.playerDidSomethingClever(0.7, 0.5);
      }
      this.state = SpawnState.SEMI_CHECKED;
      hero.setLastSpawnPoint(this.getPosition());
    }

    if (this.state === SpawnState.SEMI_CHECKED) this.frame++;
    if (this.frame > 5) this.state = SpawnState.CHECKED;
  }
}
