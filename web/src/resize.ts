// Scales the 320x240 canvas to the window. Lives in the bundle (not inline in
// index.html) so the Content-Security-Policy can forbid inline scripts.
const WIDTH = 320;
const HEIGHT = 240;

function resizeCanvas(): void {
  const c = document.getElementById('game-canvas');
  if (!c) return;
  const w = window.innerWidth;
  const h = window.innerHeight;
  // On touch devices, use fractional scaling to fill more of the screen
  const isMobile = ('ontouchstart' in window) || navigator.maxTouchPoints > 0;
  let scale: number;
  if (isMobile) {
    scale = Math.min(w / WIDTH, h / HEIGHT);
  } else {
    scale = Math.min(Math.floor(w / WIDTH), Math.floor(h / HEIGHT));
    if (scale < 1) scale = Math.min(w / WIDTH, h / HEIGHT);
  }
  c.style.width = (WIDTH * scale) + 'px';
  c.style.height = (HEIGHT * scale) + 'px';
}

resizeCanvas();
window.addEventListener('resize', resizeCanvas);
// Also resize when orientation changes on mobile
window.addEventListener('orientationchange', () => {
  setTimeout(resizeCanvas, 100);
});
