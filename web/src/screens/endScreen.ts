import { Button } from '../constants.js';
import { rgb } from '../utils.js';
import { Screen } from '../screen.js';
import { Input } from '../input.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import { Music } from '../media/music.js';
import type { BitmapFont } from '../media/font.js';

const GREEN_PEACE_DURATION = 60 * 7;

export class EndScreen extends Screen {
  private creditsOffset = 240;
  private readonly font: BitmapFont;
  private frameCounter = -60;

  constructor() {
    super();
    this.font = Resource.getFont('data/images/font.bmp');
  }

  onDraw(ctx: CanvasRenderingContext2D): void {
    ctx.fillStyle = rgb(57, 56, 41);
    ctx.fillRect(0, 0, 320, 240);

    if (this.frameCounter < 0) return;

    if (this.frameCounter < GREEN_PEACE_DURATION) {
      this.font.draw(ctx, 'THE LAST REACTOR IS IN CAPTIVITY', 50, 110);
      this.font.draw(ctx, 'THE GALAXY IS AT GREEN PEACE', 60, 120);
    } else {
      const o = this.creditsOffset;
      this.font.draw(ctx, 'CREDITS', 50, o);
      this.font.draw(ctx, 'PROGRAMMING', 50, 30 + o);
      this.font.draw(ctx, '   OLOF NAESSEN', 50, 40 + o);
      this.font.draw(ctx, '   PER LARSSON', 50, 50 + o);
      this.font.draw(ctx, '   ALEXANDER SCHRAB ', 50, 60 + o);
      this.font.draw(ctx, 'GRAPHICS', 50, 90 + o);
      this.font.draw(ctx, '   OLOF NAESSEN', 50, 100 + o);
      this.font.draw(ctx, '   TIMUR KONDRAKOV', 50, 110 + o);
      this.font.draw(ctx, '   PER LARSSON', 50, 120 + o);
      this.font.draw(ctx, 'MUSIC', 50, 150 + o);
      this.font.draw(ctx, '   OLOF NAESSEN', 50, 160 + o);
      this.font.draw(ctx, 'VOICE ACTING', 50, 190 + o);
      this.font.draw(ctx, '   PER LARSSON', 50, 200 + o);
      this.font.draw(ctx, 'NOT SHOWING UP FOR THE COMPO', 50, 230 + o);
      this.font.draw(ctx, '   TED STEEN', 50, 240 + o);
      this.font.draw(ctx, 'THIS GAME WAS DONE', 50, 270 + o);
      this.font.draw(ctx, 'FOR THE SPEEDHACK 11', 50, 280 + o);
      this.font.draw(ctx, 'COMPETITION', 50, 290 + o);
      this.font.draw(ctx, 'THANK YOU ALLEGRO.CC', 50, 300 + o);
      this.font.draw(ctx, 'FOR A GREAT COMPETITION!', 50, 310 + o);
      this.font.draw(ctx, 'THANK YOU THORBJORN LINDEIJER', 50, 340 + o);
      this.font.draw(ctx, 'FOR THE TILED EDITOR!', 50, 350 + o);
      this.font.draw(ctx, 'THANK YOU ANDERS STENBERG', 50, 380 + o);
      this.font.draw(ctx, 'FOR YOUR TILE RAYCASTING ALGORITHM!', 50, 390 + o);
      this.font.draw(ctx, 'AND THANKS FOR PLAYING', 50, 420 + o);
      this.font.draw(ctx, 'OUR GAME!', 50, 430 + o);

      if (this.creditsOffset < -450) {
        this.font.drawCenter(ctx, 'THE END', 0, 0, 320, 240);
      }
    }
  }

  onLogic(): void {
    this.frameCounter++;
    if (this.frameCounter === 0) Sound.playSample('data/sounds/green_peace');
    if (this.frameCounter === GREEN_PEACE_DURATION) {
      Music.playSong('data/music/intro2.xm');
    } else if (this.frameCounter > GREEN_PEACE_DURATION && this.frameCounter % 4 === 0) {
      this.creditsOffset--;
    }
    if (Input.isPressed(Button.EXIT)) this.exit();
  }
}
