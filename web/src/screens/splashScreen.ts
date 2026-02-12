import { rgb } from '../utils.js';
import { Screen } from '../screen.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import type { Animation } from '../media/animation.js';

export class SplashScreen extends Screen {
  private frameCounter = 0;
  private readonly logo: Animation;

  constructor() {
    super();
    this.logo = Resource.getAnimation('data/images/logo.bmp', 1);
  }

  onDraw(ctx: CanvasRenderingContext2D): void {
    let yOffset = -160 + this.frameCounter;
    if (yOffset === 0) Sound.playSample('data/sounds/boot');
    if (yOffset > 0) yOffset = 0;

    ctx.fillStyle = rgb(231, 215, 156);
    ctx.fillRect(0, 0, 320, 240);
    this.logo.drawFrame(ctx, 0,
      160 - this.logo.getFrameWidth() / 2,
      120 - this.logo.getFrameHeight() / 2 + yOffset);
  }

  onLogic(): void {
    if (this.frameCounter > 250) {
      this.exit();
      return;
    }
    this.frameCounter++;
  }
}
