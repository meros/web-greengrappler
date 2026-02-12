import { ImmutableVec2 } from '../math.js';
import { Entity } from '../entity.js';
import { GameState } from '../gameState.js';
import { GlobalOptions } from '../options.js';
import { Resource } from '../resource.js';
import { Sound } from '../media/sound.js';
import type { Animation } from '../media/animation.js';
import type { BitmapFont } from '../media/font.js';

const enum CharacterPortrait { DoctorGreen, Ted }

interface DialogueLine {
  readonly text: string;
  readonly portrait: CharacterPortrait;
}

export class Dialogue extends Entity {
  private readonly background: Animation;
  private currentCharacter = 0;
  private currentLine = 0;
  private readonly doctorGreenPortrait: Animation;
  private done = false;
  private readonly file: string;
  private readonly font: BitmapFont;
  private frameCounter = 0;
  private readonly lines: DialogueLine[] = [];
  private running = false;
  private runWithoutHero = false;
  private readonly tedPortrait: Animation;

  constructor(filename: string) {
    super();
    this.file = filename;
    this.background = Resource.getAnimation('data/images/dialogue.bmp', 1);
    this.doctorGreenPortrait = Resource.getAnimation('data/images/doctor_green_portrait.bmp', 1);
    this.tedPortrait = Resource.getAnimation('data/images/ted_portrait.bmp', 1);
    this.font = Resource.getFont('data/images/font.bmp');

    const text = Resource.getText(filename);
    const rawLines = text.split('\n');
    for (const line of rawLines) {
      if (line.length === 0) continue;
      const ch = line.charAt(0);
      const portrait = ch === 'D' ? CharacterPortrait.DoctorGreen : CharacterPortrait.Ted;
      this.lines.push({ text: line.substring(1), portrait });
    }

    this.setSize(new ImmutableVec2(10, 10 * 200));
  }

  getLayer(): number { return 3; }

  setRunWithoutHero(): void {
    this.runWithoutHero = true;
    this.running = true;
  }

  override draw(ctx: CanvasRenderingContext2D, offsetX: number, offsetY: number, _layer: number): void {
    if (this.done || !this.running) return;

    let position = 240 - this.background.getFrameHeight();
    if (GlobalOptions.dialogueAtTop()) position = 10;

    this.background.drawFrame(ctx, 0, 0, position, false, GlobalOptions.dialogueAtTop());

    const line = this.lines[this.currentLine];
    if (!line) return;

    const pX = 4;
    const pY = position + this.background.getFrameHeight() - this.tedPortrait.getFrameHeight() - 3;

    if (line.portrait === CharacterPortrait.Ted)
      this.tedPortrait.drawFrame(ctx, 0, pX, pY);
    if (line.portrait === CharacterPortrait.DoctorGreen)
      this.doctorGreenPortrait.drawFrame(ctx, 0, pX, pY);

    let fontPosition = position + this.background.getFrameHeight() - 16;
    if (GlobalOptions.dialogueAtTop()) fontPosition = position + 5;

    this.font.drawWrap(ctx, line.text, 55, fontPosition, 300, this.currentCharacter);
  }

  override update(): void {
    if (this.done) return;

    if (!this.runWithoutHero) {
      if (this.myRoom.getHero().Collides(this)) {
        this.running = true;
      }
    }

    if (!this.running) return;

    this.frameCounter++;

    const line = this.lines[this.currentLine];
    if (!line) return;

    if (this.frameCounter > 2 && this.currentCharacter < line.text.length) {
      this.currentCharacter++;
      this.frameCounter = 0;
      Sound.playSample('data/sounds/beep');
    }

    if (this.frameCounter > 90 && this.lines.length > this.currentLine) {
      this.currentLine++;
      this.currentCharacter = 0;
      this.frameCounter = 0;

      if (this.lines.length === this.currentLine) {
        const key = 'dialogue_' + this.file;
        GameState.put(key, 1);
        this.done = true;
        if (!this.runWithoutHero) this.remove();
      }
    }
  }
}
