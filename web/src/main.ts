import { TICKS_PER_SECOND, Button } from './constants.js';
import { Input } from './input.js';
import { Resource } from './resource.js';
import { ScreenManager } from './screen.js';
import { Music } from './media/music.js';
import { Sound } from './media/sound.js';
import { SplashScreen } from './screens/splashScreen.js';
import { TitleScreen } from './screens/titleScreen.js';
import { TouchControls } from './touchControls.js';

const FRAME_TIME = 1000 / TICKS_PER_SECOND;

let canvas: HTMLCanvasElement;
let ctx: CanvasRenderingContext2D;
let readyForUpdates = false;
let lastTime = 0;
let accumulator = 0;

function preloadAssets(): void {
  Resource.preLoad('data/images/boss.bmp');
  Resource.preLoad('data/images/breakinghooktile.bmp');
  Resource.preLoad('data/images/button.bmp');
  Resource.preLoad('data/images/checkpoint.bmp');
  Resource.preLoad('data/images/coin.bmp');
  Resource.preLoad('data/images/completed_level.bmp');
  Resource.preLoad('data/images/core.bmp');
  Resource.preLoad('data/images/darkbitslogo_blink.bmp');
  Resource.preLoad('data/images/darken.bmp');
  Resource.preLoad('data/images/debris.bmp');
  Resource.preLoad('data/images/dialogue.bmp');
  Resource.preLoad('data/images/doctor_green_portrait.bmp');
  Resource.preLoad('data/images/door.bmp');
  Resource.preLoad('data/images/entities.bmp');
  Resource.preLoad('data/images/fade.bmp');
  Resource.preLoad('data/images/font.bmp');
  Resource.preLoad('data/images/hand.bmp');
  Resource.preLoad('data/images/hero_fall.bmp');
  Resource.preLoad('data/images/hero_hurt.bmp');
  Resource.preLoad('data/images/hero_jump.bmp');
  Resource.preLoad('data/images/hero_run.bmp');
  Resource.preLoad('data/images/hook.bmp');
  Resource.preLoad('data/images/icons.bmp');
  Resource.preLoad('data/images/lavafill.bmp');
  Resource.preLoad('data/images/lavatop.bmp');
  Resource.preLoad('data/images/level_exit_background.bmp');
  Resource.preLoad('data/images/level_select.bmp');
  Resource.preLoad('data/images/logo.bmp');
  Resource.preLoad('data/images/movinghooktile.bmp');
  Resource.preLoad('data/images/particles.bmp');
  Resource.preLoad('data/images/reactor_shell.bmp');
  Resource.preLoad('data/images/robot.bmp');
  Resource.preLoad('data/images/rope.bmp');
  Resource.preLoad('data/images/saw.bmp');
  Resource.preLoad('data/images/selected_level_background.bmp');
  Resource.preLoad('data/images/software.bmp');
  Resource.preLoad('data/images/ted_portrait.bmp');
  Resource.preLoad('data/images/tileset1.bmp');
  Resource.preLoad('data/images/title.bmp');
  Resource.preLoad('data/images/unselected_level_background.bmp');
  Resource.preLoad('data/images/wall.bmp');
  Resource.preLoad('data/images/z_coin.bmp');

  Resource.preLoadText('data/rooms/breaktilelevel.txt');
  Resource.preLoadText('data/rooms/level1.txt');
  Resource.preLoadText('data/rooms/levellava.txt');
  Resource.preLoadText('data/rooms/olof.txt');
  Resource.preLoadText('data/rooms/olof2.txt');
  Resource.preLoadText('data/rooms/per1.txt');
  Resource.preLoadText('data/rooms/per2.txt');
  Resource.preLoadText('data/rooms/per3.txt');
  Resource.preLoadText('data/rooms/per4.txt');
  Resource.preLoadText('data/rooms/tutorial.txt');
  Resource.preLoadText('data/dialogues/1-tutorial1.txt');
  Resource.preLoadText('data/dialogues/2-tutorial2.txt');
  Resource.preLoadText('data/dialogues/boss_unlocked.txt');
  Resource.preLoadText('data/dialogues/level_select.txt');

  Sound.preload('data/sounds/alarm');
  Sound.preload('data/sounds/beep');
  Sound.preload('data/sounds/boot');
  Sound.preload('data/sounds/boss_saw');
  Sound.preload('data/sounds/coin');
  Sound.preload('data/sounds/damage');
  Sound.preload('data/sounds/green_peace');
  Sound.preload('data/sounds/hook');
  Sound.preload('data/sounds/hurt');
  Sound.preload('data/sounds/jump');
  Sound.preload('data/sounds/land');
  Sound.preload('data/sounds/no_hook');
  Sound.preload('data/sounds/reactor_explosion');
  Sound.preload('data/sounds/rope');
  Sound.preload('data/sounds/select');
  Sound.preload('data/sounds/start');
  Sound.preload('data/sounds/time');
  Sound.preload('data/sounds/timeout');
}

function postPreloadInit(): void {
  ScreenManager.add(new TitleScreen());
  ScreenManager.add(new SplashScreen());
}

function update(): void {
  ScreenManager.onLogic();
  Music.update();
  TouchControls.update();
  if (Input.isPressed(Button.FORCE_QUIT) || ScreenManager.isEmpty()) {
    // On web, just go back to title screen
    if (ScreenManager.isEmpty()) {
      ScreenManager.add(new TitleScreen());
    }
  }
}

function draw(): void {
  ctx.imageSmoothingEnabled = false;
  ScreenManager.draw(ctx);
  TouchControls.draw(ctx);
}

function gameLoop(timestamp: number): void {
  if (lastTime === 0) lastTime = timestamp;
  const delta = Math.min(timestamp - lastTime, 100); // cap delta to avoid spiral
  lastTime = timestamp;

  if (!readyForUpdates) {
    if (Resource.isDonePreloading()) {
      postPreloadInit();
      readyForUpdates = true;
    } else {
      // Draw loading screen
      ctx.fillStyle = '#393829';
      ctx.fillRect(0, 0, 320, 240);
      ctx.fillStyle = '#e7d79c';
      ctx.font = '10px monospace';
      ctx.fillText('LOADING...', 130, 120);
    }
  } else {
    accumulator += delta;
    while (accumulator >= FRAME_TIME) {
      update();
      Input.update();
      accumulator -= FRAME_TIME;
    }
    draw();
  }

  requestAnimationFrame(gameLoop);
}

function init(): void {
  canvas = document.getElementById('game-canvas') as HTMLCanvasElement;
  ctx = canvas.getContext('2d', { alpha: false })!;
  ctx.imageSmoothingEnabled = false;

  Input.init();
  TouchControls.init(canvas);
  preloadAssets();

  requestAnimationFrame(gameLoop);
}

// Enable audio on first user interaction
document.addEventListener('click', () => Sound.resumeContext(), { once: true });
document.addEventListener('keydown', () => Sound.resumeContext(), { once: true });
document.addEventListener('touchstart', () => Sound.resumeContext(), { once: true });

init();
