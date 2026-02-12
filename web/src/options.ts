export const enum VibrationType {
  SIMPLE = 'SIMPLE',
  PULSATING = 'PULSATING',
}

export type ExitCallback = () => void;

let exitCallback: ExitCallback | null = null;

export const GlobalOptions = {
  showTouchControls(): boolean { return false; },
  dialogueAtTop(): boolean { return true; },
  avoidHeroAtThumbs(): boolean { return false; },
  showFps(): boolean { return false; },

  vibrate(_time: number, _type: VibrationType): void {
    // No vibration on web
  },

  setExitCallback(cb: ExitCallback): void { exitCallback = cb; },

  exit(): void {
    if (exitCallback) exitCallback();
  },
} as const;
