import { Resource } from '../resource.js';
import type { GameImage } from '../resource.js';

export class Animation {
  public readonly frameWidth: number;
  public readonly frameHeight: number;
  private readonly frames: GameImage[];

  constructor(filename: string, numFrames?: number) {
    const allFrames = Resource.getBitmap(filename);
    this.frameHeight = allFrames.height;

    if (numFrames !== undefined) {
      this.frameWidth = Math.floor(allFrames.width / numFrames);
    } else {
      this.frameWidth = this.frameHeight;
      numFrames = Math.floor(allFrames.width / this.frameWidth);
    }

    this.frames = [];
    for (let i = 0; i < numFrames; i++) {
      this.frames.push(allFrames.subImage(i * this.frameWidth, 0, this.frameWidth, this.frameHeight));
    }
  }

  getFrame(index: number): GameImage {
    return this.frames[Math.abs(index % this.frames.length)]!;
  }

  getFrameWidth(): number { return this.frameWidth; }
  getFrameHeight(): number { return this.frameHeight; }
  getGameImage(): GameImage { return this.frames[0]!; }

  drawFrame(ctx: CanvasRenderingContext2D, frame: number, x: number, y: number,
            hFlip?: boolean, vFlip?: boolean): void {
    if (x > 320 || y > 240 || x + this.frameWidth < 0 || y + this.frameHeight < 0) return;

    const needTransform = hFlip || vFlip;
    if (needTransform) ctx.save();

    if (hFlip) {
      ctx.translate(this.frameWidth + x * 2, 0);
      ctx.scale(-1, 1);
    }
    if (vFlip) {
      ctx.translate(0, this.frameHeight + y * 2);
      ctx.scale(1, -1);
    }

    const img = this.getFrame(frame);
    img.draw(ctx, x, y);

    if (needTransform) ctx.restore();
  }
}
