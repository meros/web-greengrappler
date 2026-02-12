import { TICKS_PER_SECOND } from '../constants.js';
import { Vec2, ImmutableVec2, type ReadonlyVec2 } from '../math.js';
import { sincos } from '../utils.js';
import { Entity } from '../entity.js';
import type { Animation } from '../media/animation.js';

export class ParticleSystem extends Entity {
  private readonly animation: Animation;
  private readonly animationSpeed: number;
  private readonly blinkTimeTicks: number;
  private readonly gravity: ImmutableVec2;
  private readonly initialVel: Vec2;
  private readonly maxSpeed: number;
  private readonly minSpeed: number;
  private readonly numParticles: number;
  private readonly particlesPos: Vec2[] = [];
  private readonly particlesVel: Vec2[] = [];
  private lifeTimeTicks: number;
  private frameNum = 0;

  constructor(
    animation: Animation,
    animationSpeed: number,
    lifeTime: number,
    blinkTime: number,
    minSpeed: number,
    maxSpeed: number,
    numParticles: number,
    initialVel: ReadonlyVec2,
    gravity: number,
  ) {
    super();
    this.animation = animation;
    this.animationSpeed = animationSpeed;
    this.lifeTimeTicks = lifeTime;
    this.blinkTimeTicks = blinkTime;
    this.minSpeed = minSpeed;
    this.maxSpeed = maxSpeed;
    this.numParticles = numParticles;
    this.initialVel = new Vec2(initialVel);
    this.gravity = new ImmutableVec2(0, gravity / TICKS_PER_SECOND);
    this.setSize(new ImmutableVec2(30, 30));
  }

  getLayer(): number { return 3; }

  override onRespawn(): void { this.remove(); }

  override setPosition(pos: ReadonlyVec2): void {
    this.setPositionWithSpread(pos, 0, false);
  }

  setPositionWithSpread(position: ReadonlyVec2, randomAmount: number, randomVel: boolean): void {
    super.setPosition(position);

    for (let i = 0; i < this.numParticles; i++) {
      const pos = new Vec2(this.getPosition());
      pos.add(
        (Math.random() * 2 - 1) * randomAmount,
        (Math.random() * 2 - 1) * randomAmount,
      );
      this.particlesPos.push(pos);

      const r0710 = Math.random() * 0.3 + 0.7;
      let velocity: Vec2;
      if (randomVel) {
        velocity = new Vec2(1, 1).multiply(r0710);
      } else {
        velocity = new Vec2(sincos(2 * Math.PI * Math.random()));
      }

      const factor = Math.random() * (this.maxSpeed - this.minSpeed) + this.minSpeed;
      velocity.multiply(factor);
      velocity.add(this.initialVel).divide(TICKS_PER_SECOND);

      this.particlesVel.push(velocity);
    }
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    if (this.lifeTimeTicks < this.blinkTimeTicks && this.lifeTimeTicks % 2 === 0) return;

    let seed = 0;
    const seededRandom = (): number => {
      seed = (seed * 1103515245 + 12345) & 0x7fffffff;
      return seed;
    };

    for (let i = 0; i < this.particlesPos.length; i++) {
      const p = this.particlesPos[i]!;
      this.animation.drawFrame(
        ctx,
        Math.abs(Math.floor(this.frameNum / this.animationSpeed) + seededRandom()),
        Math.round(p.x + offsetX - this.animation.getFrameWidth() / 2),
        Math.round(p.y + offsetY - this.animation.getFrameHeight() / 2),
      );
    }
  }

  override update(): void {
    this.frameNum++;
    if (this.lifeTimeTicks > 0) {
      for (let i = 0; i < this.particlesVel.length; i++) {
        this.particlesPos[i]!.add(this.particlesVel[i]!);
        this.particlesVel[i]!.add(this.gravity);
      }
      this.lifeTimeTicks--;
    } else {
      this.remove();
    }
  }
}
