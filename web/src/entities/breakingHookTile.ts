import { ImmutableVec2 } from '../math.js';
import { lerp } from '../utils.js';
import { Entity } from '../entity.js';
import { PlayerSkill } from '../playerSkill.js';
import { Resource } from '../resource.js';
import type { Animation } from '../media/animation.js';
import type { Room } from '../room.js';
import { RopeState } from './hero.js';
import { ParticleSystem } from './particleSystem.js';

const MAX_BREAK_FRAMES = 50;
const MIN_BREAK_FRAMES = 30;

export class BreakingHookTile extends Entity {
  private breakCounter = 0;
  private breaking = false;
  private destroyed = false;
  private readonly sprite: Animation;
  private tileX = 0;
  private tileY = 0;

  constructor() {
    super();
    this.sprite = Resource.getAnimation('data/images/breakinghooktile.bmp');
    this.setSize(new ImmutableVec2(this.sprite.getFrameWidth(), this.sprite.getFrameHeight()));
  }

  getLayer(): number { return 3; }

  override setRoom(room: Room): void {
    super.setRoom(room);
    this.onRespawn();
  }

  override onRespawn(): void {
    this.tileX = Math.floor(this.getPosition().x / this.myRoom.getTileWidth());
    this.tileY = Math.floor(this.getPosition().y / this.myRoom.getTileHeight());
    this.myRoom.setHookable(this.tileX, this.tileY, true);
    this.myRoom.setCollidable(this.tileX, this.tileY, true);
    this.breakCounter = 0;
    this.breaking = false;
    this.destroyed = false;
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    if (this.destroyed) return;
    const x = this.getDrawPositionX() + offsetX - this.sprite.getFrameWidth() / 2;
    const y = this.getDrawPositionY() + offsetY - this.sprite.getFrameWidth() / 2;
    if (!this.breaking || (this.breakCounter & 1) === 0) {
      this.sprite.drawFrame(ctx, 0, x, y);
    }
  }

  override update(): void {
    if (this.destroyed) return;

    if (this.breaking) {
      this.breakCounter++;
      let breakFrames = lerp(MAX_BREAK_FRAMES, MIN_BREAK_FRAMES, PlayerSkill.get());

      if (this.myRoom.getHero().getRopeState() !== RopeState.Attached) {
        breakFrames = -1;
      }

      if (this.breakCounter >= breakFrames) {
        this.destroyed = true;
        this.myRoom.setHookable(this.tileX, this.tileY, false);
        this.myRoom.setCollidable(this.tileX, this.tileY, false);

        const ps = new ParticleSystem(
          Resource.getAnimation('data/images/debris.bmp', 4),
          10, 50, 20, 1, 50, 2, new ImmutableVec2(0, -30), 2.0);
        ps.setPositionWithSpread(this.getPosition(), 2, false);
        this.myRoom.addEntity(ps);
      }
    }

    const hero = this.myRoom.getHero();
    if (hero.getRopeState() === RopeState.Attached) {
      const ropeTileX = Math.floor(hero.getRopePosition().x / this.myRoom.getTileWidth());
      const ropeTileY = Math.floor(hero.getRopePosition().y / this.myRoom.getTileWidth());
      if (ropeTileX === this.tileX && ropeTileY === this.tileY) {
        this.breaking = true;
      }
    }
  }
}
