import { Direction } from '../constants.js';
import { Vec2, ImmutableVec2, type ReadonlyVec2 } from '../math.js';
import { sincos } from '../utils.js';
import { Entity } from '../entity.js';
import { PlayerSkill } from '../playerSkill.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import type { Animation } from '../media/animation.js';
import type { Room } from '../room.js';

const COIN_SIZE = 12;

const enum CoinType { DYNAMIC, STATIC }

export class Coin extends Entity {
  private readonly animationCoin: Animation;
  private frame = 0;
  private lifeTime = 0;
  private temporary = false;
  private type: CoinType = CoinType.STATIC;
  private collides = false;

  static spawnDeathCoins(count: number, center: ReadonlyVec2, lifeTime: number, room: Room): void {
    for (let i = 0; i < count; i++) {
      const vel = sincos(Math.PI * (i + 1) / (count + 1));
      const velocity = new ImmutableVec2(vel.x * 100, vel.y * -200);
      const coin = new Coin();
      coin.setLifeTime(lifeTime);
      coin.setPosition(center);
      coin.setVelocity(velocity);
      room.addEntity(coin);
    }
  }

  constructor() {
    super();
    this.animationCoin = Resource.getAnimation('data/images/coin.bmp', 4);
    this.setSize(new ImmutableVec2(COIN_SIZE, COIN_SIZE));
  }

  getLayer(): number { return 3; }

  setLifeTime(lt: number): void {
    this.lifeTime = lt;
    this.type = CoinType.DYNAMIC;
    this.temporary = lt !== 0;
  }

  setCollides(): void { this.collides = true; }
  isDynamic(): boolean { return this.type === CoinType.DYNAMIC; }

  override onRespawn(): void {
    if (this.type === CoinType.DYNAMIC) this.remove();
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    let x = this.getPosition().x - this.animationCoin.getFrameWidth() / 2 + offsetX;
    let y = this.getPosition().y - this.animationCoin.getFrameHeight() / 2 + offsetY;

    if (!this.temporary || Math.floor(this.lifeTime / 10) % 2 === 0) {
      let f = this.frame;
      if (f > 180) f = 0;
      this.animationCoin.drawFrame(ctx, (f < 5 * 4) ? Math.floor(f / 5) : 0,
        Math.round(x), Math.round(y), false, false);
    }
  }

  override update(): void {
    if (this.type === CoinType.DYNAMIC) {
      if (this.myRoom.getHero().Collides(this)) {
        this.setCollides();
      }

      if (this.temporary && this.lifeTime === 0) {
        this.remove();
      } else {
        this.myVelocity.add(0, 6.0);
        const bumps = this.moveWithCollision();

        if (bumps.has(Direction.LEFT) || bumps.has(Direction.RIGHT)) {
          if (Math.abs(this.myVelocity.x) > 10) Sound.playSample('data/sounds/coin');
          this.myVelocity.set(this.myVelocity.x * -0.5, this.myVelocity.y);
        }
        if (bumps.has(Direction.UP) || bumps.has(Direction.DOWN)) {
          if (Math.abs(this.myVelocity.y) > 10) Sound.playSample('data/sounds/coin');
          this.myVelocity.set(this.myVelocity.x, this.myVelocity.y * -0.2);
        }
        this.lifeTime--;
      }
    }

    if (this.collides) {
      if (this.myRoom.getHero().gotCoin()) {
        PlayerSkill.playerDidSomethingClever(0.3, 0.05);
        Sound.playSample('data/sounds/coin');
        this.remove();
      }
    }

    this.frame++;
  }
}
