import { Button } from '../constants.js';
import { Screen, ScreenManager } from '../screen.js';
import { Input } from '../input.js';
import { GameState } from '../gameState.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import { Music } from '../media/music.js';
import { selectFeedback } from '../utils.js';
import type { Animation } from '../media/animation.js';
import type { BitmapFont } from '../media/font.js';
import { LevelDescription } from '../levelDescription.js';
import { LevelSelectScreen } from './levelSelectScreen.js';
import { LevelScreen } from './levelScreen.js';

export class TitleScreen extends Screen {
  private hasContinue = false;
  private readonly font: BitmapFont;
  private frameCounter = 0;
  private gameStart = false;
  private readonly hand: Animation;
  private selected = 0;
  private readonly title: Animation;

  constructor() {
    super();
    this.font = Resource.getFont('data/images/font.bmp');
    this.hand = Resource.getAnimation('data/images/hand.bmp', 1);
    this.title = Resource.getAnimation('data/images/title.bmp', 1);
  }

  override onEntered(): void {
    this.gameStart = false;
    Music.playSong('data/music/intro2.xm');
    this.hasContinue = GameState.isSavePresent();
    if (this.hasContinue) this.selected = 1;
  }

  onDraw(ctx: CanvasRenderingContext2D): void {
    this.title.drawFrame(ctx, 0, 0, 0);

    // Draw controls help text
    if (Input.hasTouch()) {
      this.font.drawCenter(ctx, 'TOUCH SCREEN TO SHOW CONTROLS', 0, 115, 320, 10);
    } else {
      this.font.drawCenter(ctx, 'ARROWS-MOVE SPACE-JUMP CTRL-ROPE', 0, 115, 320, 10);
    }

    if (!this.gameStart)
      this.hand.drawFrame(ctx, 0, 115, 150 + this.selected * 10);

    if (!this.gameStart || (this.selected !== 0 || this.frameCounter % 10 < 5))
      this.font.draw(ctx, 'NEW GAME', 126, 150);

    if (this.hasContinue) {
      if (!this.gameStart || (this.selected !== 1 || this.frameCounter % 10 < 5))
        this.font.draw(ctx, 'CONTINUE', 126, 160);
      this.font.draw(ctx, 'EXIT GAME', 126, 170);
    } else {
      this.font.draw(ctx, 'EXIT GAME', 126, 160);
    }

    this.font.draw(ctx, 'DARKBITS', 130, 215);
    this.font.draw(ctx, 'SPEEDHACK 2011', 113, 225);
  }

  onLogic(): void {
    this.frameCounter++;

    if (this.gameStart) {
      if (this.frameCounter > 100) {
        ScreenManager.add(new LevelSelectScreen());
        if (this.selected === 0) {
          ScreenManager.add(new LevelScreen(new LevelDescription(
            'tutorial', 'data/rooms/tutorial.txt', 0, 'data/music/olof9.xm')));
        }
      }
      return;
    }

    if (Input.isPressed(Button.DOWN)) {
      this.selected++;
      if (this.selected > 1 && !this.hasContinue) this.selected = 1;
      else if (this.selected > 2) this.selected = 2;
      else selectFeedback();
    }

    if (Input.isPressed(Button.UP)) {
      this.selected--;
      if (this.selected < 0) this.selected = 0;
      else selectFeedback();
    }

    if (Input.isPressed(Button.EXIT)) this.exit();

    if (Input.isPressed(Button.FIRE)) {
      if (this.selected === 0) {
        this.gameStart = true;
        this.frameCounter = 0;
        Music.stop();
        Sound.playSample('data/sounds/start');
        GameState.clear();
      }
      if (this.selected === 1 && this.hasContinue) {
        this.gameStart = true;
        this.frameCounter = 0;
        Music.stop();
        Sound.playSample('data/sounds/start');
        GameState.loadFromFile();
      }
      if (this.selected === 1 && !this.hasContinue) this.exit();
      if (this.selected === 2 && this.hasContinue) this.exit();
    }
  }
}
