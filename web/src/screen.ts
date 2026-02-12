export abstract class Screen {
  abstract onDraw(ctx: CanvasRenderingContext2D): void;
  abstract onLogic(): void;

  onEntered(): void {}
  onExited(): void {}
  onEnter(_ctx: CanvasRenderingContext2D): boolean { return true; }
  onExit(_ctx: CanvasRenderingContext2D): boolean { return true; }

  exit(): void {
    ScreenManager.exit(this);
  }

  isTop(): boolean {
    return ScreenManager.getTop() === this;
  }
}

export const ScreenManager = {
  _stack: [] as Screen[],
  _screenToEnter: null as Screen | null,
  _screenToExit: null as Screen | null,

  add(screen: Screen): void {
    this._stack.push(screen);
    this._screenToEnter = screen;
  },

  exit(screen: Screen): void {
    if (this.getTop() !== screen) return;
    if (this._screenToExit !== null) return;
    this._screenToExit = screen;
  },

  getTop(): Screen | null {
    return this._stack.length > 0 ? this._stack[this._stack.length - 1]! : null;
  },

  isEmpty(): boolean { return this._stack.length === 0; },

  onLogic(): void {
    if (this._screenToExit === null && this._screenToEnter === null) {
      this.getTop()?.onLogic();
    }
  },

  draw(ctx: CanvasRenderingContext2D): void {
    if (this._screenToExit === null && this._screenToEnter === null) {
      this.getTop()?.onDraw(ctx);
    }

    if (this._screenToExit !== null) {
      const exitDone = this._screenToExit.onExit(ctx);
      if (!exitDone) return;
      const idx = this._stack.indexOf(this._screenToExit);
      if (idx >= 0) this._stack.splice(idx, 1);
      this._screenToExit.onExited();
      this._screenToExit = null;
      if (this.getTop() !== null) this._screenToEnter = this.getTop();
    }

    if (this._screenToEnter !== null) {
      const enterDone = this._screenToEnter.onEnter(ctx);
      if (!enterDone) return;
      this._screenToEnter.onEntered();
      this._screenToEnter = null;
    }
  },
};
