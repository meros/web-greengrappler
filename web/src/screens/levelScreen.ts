import { Button } from '../constants.js';
import { rgb, selectFeedback } from '../utils.js';
import { Screen } from '../screen.js';
import { Input } from '../input.js';
import { GameState } from '../gameState.js';
import { Resource } from '../resource.js';
import { Music } from '../media/music.js';
import type { Animation } from '../media/animation.js';
import type { BitmapFont } from '../media/font.js';
import { LevelDescription } from '../levelDescription.js';
import { Room } from '../room.js';
import { loadRoom } from '../roomLoader.js';
import { LevelSelectScreen } from './levelSelectScreen.js';

export class LevelScreen extends Screen {
  private readonly darken: Animation;
  private isExit = false;
  private readonly exitBg: Animation;
  private readonly font: BitmapFont;
  private readonly hand: Animation;
  private readonly levelDesc: LevelDescription;
  private readonly room: Room;
  private selected = 0;

  constructor(desc: LevelDescription) {
    super();
    this.levelDesc = desc;
    this.darken = Resource.getAnimation('data/images/darken.bmp');
    this.exitBg = Resource.getAnimation('data/images/level_exit_background.bmp', 1);
    this.font = Resource.getFont('data/images/font.bmp');
    this.hand = Resource.getAnimation('data/images/hand.bmp', 1);

    Music.playSong(desc.musicFile);
    this.room = loadRoom(desc.levelFile);
  }

  onDraw(ctx: CanvasRenderingContext2D): void {
    ctx.fillStyle = rgb(57, 56, 41);
    ctx.fillRect(0, 0, 320, 240);

    this.room.onDraw(ctx);

    const coins = GameState.getInt('coins');
    const coinsStr = '[x' + coins;

    ctx.fillStyle = rgb(57, 56, 41);
    ctx.fillRect(0, 0, 320, 10);
    this.font.draw(ctx, coinsStr, 1, 1);

    if (this.isExit) {
      for (let y = 0; y < 15; y++)
        for (let x = 0; x < 20; x++)
          this.darken.drawFrame(ctx, 0, x * 16, y * 16);

      this.exitBg.drawFrame(ctx, 0,
        160 - this.exitBg.getFrameWidth() / 2,
        120 - this.exitBg.getFrameHeight() / 2);
      this.font.draw(ctx, 'CONTINUE', 130, 110);
      this.font.draw(ctx, 'EXIT LEVEL', 130, 120);
      this.hand.drawFrame(ctx, 0, 120, 110 + this.selected * 10);
    }
  }

  onLogic(): void {
    if (this.room.isCompleted()) {
      if (this.levelDesc.levelFile === 'data/rooms/olof.txt')
        LevelSelectScreen.bossLevelCompleted = true;
      GameState.put(this.levelDesc.levelFile, 1);
      this.exit();
    }

    if (!this.isExit) this.room.onLogic();

    if (Input.isPressed(Button.EXIT) && !this.isExit) {
      selectFeedback();
      this.isExit = true;
    } else if (Input.isPressed(Button.EXIT) && this.isExit) {
      selectFeedback();
      this.isExit = false;
    }

    if (!this.isExit) return;

    if (Input.isPressed(Button.DOWN)) {
      this.selected++;
      if (this.selected > 1) this.selected = 1;
      else selectFeedback();
    }
    if (Input.isPressed(Button.UP)) {
      this.selected--;
      if (this.selected < 0) this.selected = 0;
      else selectFeedback();
    }
    if (Input.isPressed(Button.FIRE)) {
      selectFeedback();
      if (this.selected === 0) this.isExit = false;
      else if (this.selected === 1) this.exit();
    }
  }
}
