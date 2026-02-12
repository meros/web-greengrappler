import { Button } from '../constants.js';
import { rgb, selectFeedback } from '../utils.js';
import { Screen, ScreenManager } from '../screen.js';
import { Input } from '../input.js';
import { GameState } from '../gameState.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import { Music } from '../media/music.js';
import type { Animation } from '../media/animation.js';
import type { BitmapFont } from '../media/font.js';
import { LevelDescription } from '../levelDescription.js';
import { Dialogue } from '../entities/dialogue.js';
import { LevelScreen } from './levelScreen.js';
import { EndScreen } from './endScreen.js';

interface Particle { x: number; y: number; z: number; }

export class LevelSelectScreen extends Screen {
  static bossLevelCompleted = false;

  private readonly background: Animation;
  private bossLevelUnlocked = false;
  private readonly bossUnlockedDialogue: Dialogue;
  private readonly completedIcon: Animation;
  private readonly firstDialogue: Dialogue;
  private readonly font: BitmapFont;
  private frameCounter = 0;
  private readonly icons: Animation;
  private readonly levelDescs: LevelDescription[] = new Array<LevelDescription>(9);
  private levelSelected = false;
  private levelSelectedCounter = 0;
  private readonly particles: Particle[] = [];
  private playingBossLevel = false;
  private runBossUnlockedDialogue = false;
  private runFirstDialogue = false;
  private readonly selectedBg: Animation;
  private selectedX = 0;
  private selectedY = 0;
  private readonly unselectedBg: Animation;

  constructor() {
    super();
    this.background = Resource.getAnimation('data/images/level_select.bmp', 1);
    this.completedIcon = Resource.getAnimation('data/images/completed_level.bmp', 1);
    this.font = Resource.getFont('data/images/font.bmp');
    this.icons = Resource.getAnimation('data/images/icons.bmp', 9);
    this.selectedBg = Resource.getAnimation('data/images/selected_level_background.bmp', 6);
    this.unselectedBg = Resource.getAnimation('data/images/unselected_level_background.bmp', 1);

    for (let i = 0; i < 200; i++) {
      this.particles.push({ x: Math.random() * 320, y: Math.random() * 240, z: Math.random() * 3 + 1 });
    }

    this.setLevel(0, 0, new LevelDescription('GRAPPLIN\' HEAVEN', 'data/rooms/olof2.txt', 0, 'data/music/olof9.xm'));
    this.setLevel(1, 0, new LevelDescription('CAVERNOUS', 'data/rooms/per4.txt', 1, 'data/music/olof8.xm'));
    this.setLevel(2, 0, new LevelDescription('HARRY U.P.', 'data/rooms/level1.txt', 7, 'data/music/olof2-rmx.xm'));
    this.setLevel(0, 1, new LevelDescription('GLIDER', 'data/rooms/per1.txt', 3, 'data/music/olof12.xm'));
    this.setLevel(1, 1, new LevelDescription('FINAL CORE', 'data/rooms/olof.txt', 5, 'data/music/spooky.xm'));
    this.setLevel(2, 1, new LevelDescription('UNSTABLE', 'data/rooms/breaktilelevel.txt', 4, 'data/music/olof8.xm'));
    this.setLevel(0, 2, new LevelDescription('THE TOWER', 'data/rooms/per2.txt', 2, 'data/music/olof12.xm'));
    this.setLevel(1, 2, new LevelDescription('WALL OF DEATH', 'data/rooms/per3.txt', 6, 'data/music/olof2-rmx.xm'));
    this.setLevel(2, 2, new LevelDescription('LAVA LAND', 'data/rooms/levellava.txt', 8, 'data/music/olof2-rmx.xm'));

    this.firstDialogue = new Dialogue('data/dialogues/level_select.txt');
    this.firstDialogue.setRunWithoutHero();
    this.bossUnlockedDialogue = new Dialogue('data/dialogues/boss_unlocked.txt');
    this.bossUnlockedDialogue.setRunWithoutHero();
  }

  private setLevel(x: number, y: number, desc: LevelDescription): void {
    this.levelDescs[y * 3 + x] = desc;
  }

  override onEntered(): void {
    GameState.saveToFile();

    this.bossLevelUnlocked = true;
    for (let i = 0; i < 9; i++) {
      if (i === 4) continue;
      if (GameState.getInt(this.levelDescs[i]!.levelFile) === 0) {
        this.bossLevelUnlocked = false;
        break;
      }
    }

    const bossLevel = this.levelDescs[4]!;
    if (GameState.getInt(bossLevel.levelFile) === 1 && this.playingBossLevel && LevelSelectScreen.bossLevelCompleted) {
      LevelSelectScreen.bossLevelCompleted = false;
      this.playingBossLevel = false;
      ScreenManager.add(new EndScreen());
      return;
    }

    this.levelSelected = false;
    this.levelSelectedCounter = 0;
    Music.playSong('data/music/level_select.xm');

    if (GameState.getInt('level_select_dialogue') === 0) {
      this.runFirstDialogue = true;
      GameState.put('level_select_dialogue', 1);
    }

    if (this.bossLevelUnlocked && GameState.getInt('level_boss_unlocked_dialogue') === 0) {
      this.runBossUnlockedDialogue = true;
      GameState.put('level_boss_unlocked_dialogue', 1);
    }
  }

  override onExited(): void {
    GameState.saveToFile();
  }

  onDraw(ctx: CanvasRenderingContext2D): void {
    ctx.fillStyle = rgb(57, 56, 41);
    ctx.fillRect(0, 0, 320, 240);

    for (const p of this.particles) {
      ctx.fillStyle = p.z <= 2 ? rgb(123, 113, 99) : rgb(181, 166, 107);
      ctx.fillRect(Math.round(p.x), Math.round(p.y), 2, 1);
    }

    if (!this.levelSelected) this.background.drawFrame(ctx, 0, 0, 0);

    if (this.levelSelected && this.levelSelectedCounter % 6 < 3 && this.levelSelectedCounter < 20) {
      ctx.fillStyle = rgb(231, 215, 156);
      ctx.fillRect(0, 0, 320, 240);
    }

    if (this.levelSelected && this.levelSelectedCounter < 10) return;

    if (!this.levelSelected) {
      this.font.drawCenter(ctx, 'SELECT LEVEL', 0, 30, 320, 20);
    }

    const selIdx = this.selectedY * 3 + this.selectedX;

    for (let i = 0; i < 9; i++) {
      const x = i % 3;
      const y = Math.floor(i / 3);
      const desc = this.levelDescs[i]!;

      const anim = (this.selectedX === x && this.selectedY === y) ? this.selectedBg : this.unselectedBg;
      const xPos = (x - 1) * (anim.getFrameWidth() + 5) - anim.getFrameWidth() / 2;
      const yPos = (y - 1) * (anim.getFrameHeight() + 5) - anim.getFrameHeight() / 2;

      anim.drawFrame(ctx, Math.floor(this.frameCounter / 3), 160 + xPos, 120 + yPos);

      if (!this.levelSelected || selIdx === i) {
        if (i === 4 && this.bossLevelUnlocked || i !== 4)
          this.icons.drawFrame(ctx, desc.frameIndex, 164 + xPos, 124 + yPos);
      }
      if (!this.levelSelected && GameState.getInt(desc.levelFile) === 1) {
        this.completedIcon.drawFrame(ctx, 0, 164 + xPos, 124 + yPos);
      }
    }

    if (!this.levelSelected) {
      const desc = this.levelDescs[selIdx]!;
      this.font.draw(ctx, desc.name, 220, 68);

      if (GameState.getInt(desc.levelFile) === 1) {
        this.font.draw(ctx, 'STATUS:', 220, 88);
        this.font.draw(ctx, '   CLEARED', 220, 98);
      } else if (selIdx === 4 && !this.bossLevelUnlocked) {
        this.font.draw(ctx, 'STATUS:', 220, 88);
        this.font.draw(ctx, '   LOCKED', 220, 98);
      } else {
        this.font.draw(ctx, 'STATUS:', 220, 88);
        this.font.draw(ctx, '   RADIOACTIVE', 220, 98);
      }
    }

    if (this.runFirstDialogue) this.firstDialogue.draw(ctx, 0, 0, 0);
    if (this.runBossUnlockedDialogue) this.bossUnlockedDialogue.draw(ctx, 0, 0, 0);
  }

  onLogic(): void {
    this.frameCounter++;

    for (const p of this.particles) {
      p.x -= p.z / 2;
      if (p.x < 0) {
        p.x = 320;
        p.y = Math.random() * 240;
        p.z = Math.random() * 3 + 1;
      }
    }

    if (this.levelSelected) {
      this.levelSelectedCounter++;
      if (this.levelSelectedCounter > 100) {
        const index = this.selectedY * 3 + this.selectedX;
        this.playingBossLevel = index === 4;
        ScreenManager.add(new LevelScreen(this.levelDescs[index]!));
      }
      return;
    }

    if (Input.isPressed(Button.EXIT)) { this.exit(); selectFeedback(); }

    if (Input.isPressed(Button.LEFT)) {
      this.selectedX--;
      if (this.selectedX < 0) this.selectedX = 0;
      else selectFeedback();
    }
    if (Input.isPressed(Button.RIGHT)) {
      this.selectedX++;
      if (this.selectedX > 2) this.selectedX = 2;
      else selectFeedback();
    }
    if (Input.isPressed(Button.UP)) {
      this.selectedY--;
      if (this.selectedY < 0) this.selectedY = 0;
      else selectFeedback();
    }
    if (Input.isPressed(Button.DOWN)) {
      this.selectedY++;
      if (this.selectedY > 2) this.selectedY = 2;
      else selectFeedback();
    }

    if (Input.isPressed(Button.FIRE)) {
      const index = this.selectedY * 3 + this.selectedX;
      if ((index === 4 && this.bossLevelUnlocked) || index !== 4) {
        Music.stop();
        Sound.playSample('data/sounds/start');
        this.levelSelected = true;
        this.levelSelectedCounter = 0;
      }
    }

    if (this.runFirstDialogue) this.firstDialogue.update();
    if (this.runBossUnlockedDialogue) this.bossUnlockedDialogue.update();
  }
}
