/**
 * Touch controls integration tests for Green Grappler.
 * Verifies: touch detection, virtual controls overlay, d-pad, action buttons,
 * help text fade, landscape support, multi-touch.
 *
 * Usage: node touch.test.mjs <game-url>
 * Requires: puppeteer-core, Google Chrome/Chromium
 */
import puppeteer from 'puppeteer-core';
import { existsSync } from 'fs';

const GAME_URL = process.argv[2] || 'http://localhost:8080';
const CHROME = process.env.CHROME_PATH || findChrome();
let passed = 0;
let failed = 0;

function findChrome() {
  const candidates = [
    '/usr/bin/google-chrome',
    '/usr/bin/chromium',
    '/usr/bin/chromium-browser',
    '/etc/profiles/per-user/meros/bin/google-chrome',
  ];
  for (const c of candidates) {
    if (existsSync(c)) return c;
  }
  return 'google-chrome';
}

function assert(name, condition, detail) {
  if (condition) {
    console.log(`  PASS: ${name}`);
    passed++;
  } else {
    console.log(`  FAIL: ${name}${detail ? ' - ' + detail : ''}`);
    failed++;
  }
}

async function sleep(ms) { return new Promise(r => setTimeout(r, ms)); }

// Dispatch a touch event at viewport coordinates via CDP
async function dispatchTouch(cdp, type, points) {
  await cdp.send('Input.dispatchTouchEvent', {
    type,
    touchPoints: points.map((p, i) => ({ x: p.x, y: p.y, id: i })),
  });
}

// Get the canvas bounding rect in viewport coordinates
async function getCanvasRect(page) {
  return page.evaluate(() => {
    const c = document.getElementById('game-canvas');
    const r = c.getBoundingClientRect();
    return { left: r.left, top: r.top, width: r.width, height: r.height };
  });
}

// Convert canvas-relative fraction (0-1) to viewport coordinates
function canvasToViewport(rect, fracX, fracY) {
  return {
    x: Math.round(rect.left + rect.width * fracX),
    y: Math.round(rect.top + rect.height * fracY),
  };
}

// Hash a region of the canvas for comparison
async function canvasHash(page, x, y, w, h) {
  return page.evaluate(({x, y, w, h}) => {
    const c = document.getElementById('game-canvas');
    const ctx = c.getContext('2d');
    const data = ctx.getImageData(x, y, w, h).data;
    let hash = 0;
    for (let i = 0; i < data.length; i += 16) {
      hash = ((hash << 5) - hash + data[i]) | 0;
    }
    return hash;
  }, {x, y, w, h});
}

async function run() {
  console.log(`Testing touch controls at ${GAME_URL} with Chrome at ${CHROME}\n`);

  const browser = await puppeteer.launch({
    headless: 'new',
    executablePath: CHROME,
    protocolTimeout: 60000,
    args: ['--no-sandbox', '--disable-setuid-sandbox', '--disable-gpu'],
  });

  const page = await browser.newPage();

  // Emulate a mobile device in landscape with touch
  await page.emulate({
    viewport: { width: 800, height: 480, isMobile: true, hasTouch: true, deviceScaleFactor: 2 },
    userAgent: 'Mozilla/5.0 (Linux; Android 13) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0 Mobile Safari/537.36',
  });

  const pageErrors = [];
  page.on('pageerror', err => pageErrors.push(err.message));

  const failedRequests = [];
  page.on('response', resp => {
    if (resp.status() >= 400 && !resp.url().includes('favicon')) {
      failedRequests.push(`${resp.url()} -> ${resp.status()}`);
    }
  });

  // --- Test 1: Page loads on mobile ---
  console.log('1. Loading game on mobile viewport...');
  await page.goto(GAME_URL, { waitUntil: 'domcontentloaded', timeout: 15000 });
  assert('Page loads on mobile', true);

  // --- Test 2: Canvas exists and scales properly ---
  const canvasInfo = await page.evaluate(() => {
    const c = document.getElementById('game-canvas');
    return c ? { width: c.width, height: c.height,
      cssWidth: parseFloat(c.style.width), cssHeight: parseFloat(c.style.height) } : null;
  });
  assert('Canvas exists on mobile', canvasInfo !== null);
  assert('Canvas maintains 320x240 internal resolution',
    canvasInfo?.width === 320 && canvasInfo?.height === 240);

  // --- Test 3: Wait for title screen to render ---
  console.log('\n2. Waiting for assets to load...');
  await sleep(6000);

  assert('No JS errors during load', pageErrors.length === 0,
    pageErrors.length > 0 ? pageErrors.join('; ') : undefined);

  // --- Test 4: Help text visible on load (touch-capable device) ---
  console.log('\n3. Checking help text on load...');
  // Help text should be drawn on the canvas for touch devices
  // Check the bottom-left area (d-pad region) and bottom-right area (buttons) for text
  const helpTextPixels = await page.evaluate(() => {
    const c = document.getElementById('game-canvas');
    const ctx = c.getContext('2d');
    // Check the area where "MOVE" help text should appear (near d-pad, around y=138-148)
    const dpadArea = ctx.getImageData(30, 130, 60, 20).data;
    // Check the area where "JUMP" help text should appear (near jump btn, around y=175-185)
    const jumpArea = ctx.getImageData(190, 168, 60, 20).data;
    let dpadBright = 0, jumpBright = 0;
    for (let i = 0; i < dpadArea.length; i += 4) {
      if (dpadArea[i] > 100 || dpadArea[i+1] > 100 || dpadArea[i+2] > 100) dpadBright++;
    }
    for (let i = 0; i < jumpArea.length; i += 4) {
      if (jumpArea[i] > 100 || jumpArea[i+1] > 100 || jumpArea[i+2] > 100) jumpBright++;
    }
    return { dpadBright, jumpBright };
  });
  assert('Help text visible near d-pad on load', helpTextPixels.dpadBright > 5,
    `${helpTextPixels.dpadBright} bright pixels in d-pad label area`);
  assert('Help text visible near jump button on load', helpTextPixels.jumpBright > 5,
    `${helpTextPixels.jumpBright} bright pixels in jump label area`);

  // --- Test 5: Help text fades out ---
  console.log('\n4. Checking help text fades...');
  await sleep(6000); // Wait for help text to fully fade (3s show + 2s fade)
  const helpTextAfterFade = await page.evaluate(() => {
    const c = document.getElementById('game-canvas');
    const ctx = c.getContext('2d');
    const dpadArea = ctx.getImageData(30, 130, 60, 20).data;
    let dpadBright = 0;
    for (let i = 0; i < dpadArea.length; i += 4) {
      if (dpadArea[i] > 100 || dpadArea[i+1] > 100 || dpadArea[i+2] > 100) dpadBright++;
    }
    return dpadBright;
  });
  assert('Help text faded out', helpTextAfterFade < helpTextPixels.dpadBright,
    `before=${helpTextPixels.dpadBright}, after=${helpTextAfterFade}`);

  // --- Test 6: Start game ---
  console.log('\n5. Starting game (Enter to select)...');
  await page.keyboard.press('Enter');
  await sleep(2000);

  // Dismiss dialogues
  for (let i = 0; i < 20; i++) {
    await page.keyboard.press('Enter');
    await sleep(200);
  }
  await sleep(500);

  // --- Test 7: Touch activates controls overlay ---
  console.log('\n6. Testing touch activates controls overlay...');
  const rect = await getCanvasRect(page);
  const cdp = await page.createCDPSession();

  // Snapshot before touch
  const beforeTouch = await canvasHash(page, 260, 150, 60, 90);

  // Touch the center of the canvas (should activate overlay)
  const centerPt = canvasToViewport(rect, 0.5, 0.5);
  await dispatchTouch(cdp, 'touchStart', [centerPt]);
  await sleep(100);
  await dispatchTouch(cdp, 'touchEnd', []);
  await sleep(100);

  // Check if the overlay appeared in the button area (bottom-right region)
  const afterTouch = await canvasHash(page, 260, 150, 60, 90);
  // The overlay should have changed the appearance of the button region
  // (controls.png draws buttons there at 60% alpha)
  assert('Touch activates controls overlay',
    beforeTouch !== afterTouch,
    `hash before=${beforeTouch}, after=${afterTouch}`);

  // --- Test 8: D-pad right touch moves hero ---
  console.log('\n7. Testing d-pad right touch...');
  const beforeDpad = await canvasHash(page, 0, 20, 320, 220);

  // D-pad right is at approx 68-122 in canvas X, 118-240 in canvas Y
  // In viewport coords: fracX ~ 0.3, fracY ~ 0.75
  const dpadRight = canvasToViewport(rect, 0.28, 0.75);
  await dispatchTouch(cdp, 'touchStart', [dpadRight]);
  await sleep(2000);
  await dispatchTouch(cdp, 'touchEnd', []);
  await sleep(500);

  const afterDpad = await canvasHash(page, 0, 20, 320, 220);
  assert('D-pad right touch moves hero', beforeDpad !== afterDpad);

  // --- Test 9: Jump button touch ---
  console.log('\n8. Testing jump button touch...');
  const beforeJump = await canvasHash(page, 0, 20, 320, 220);

  // Jump button is at approx 205-263 in canvas X, 155-233 in canvas Y
  // In viewport coords: fracX ~ 0.73, fracY ~ 0.8
  const jumpBtn = canvasToViewport(rect, 0.73, 0.8);
  await dispatchTouch(cdp, 'touchStart', [jumpBtn]);
  await sleep(100);
  await dispatchTouch(cdp, 'touchEnd', []);
  await sleep(1000);

  const afterJump = await canvasHash(page, 0, 20, 320, 220);
  assert('Jump touch changes screen (hero jumps)', beforeJump !== afterJump);

  // --- Test 10: Fire/rope button touch ---
  console.log('\n9. Testing fire/rope button touch...');
  const beforeFire = await canvasHash(page, 0, 20, 320, 220);

  // Fire button is at approx 263-320 in canvas X, 155-233 in canvas Y
  // In viewport coords: fracX ~ 0.91, fracY ~ 0.8
  const fireBtn = canvasToViewport(rect, 0.91, 0.8);
  await dispatchTouch(cdp, 'touchStart', [fireBtn]);
  await sleep(100);
  await dispatchTouch(cdp, 'touchEnd', []);
  await sleep(1000);

  const afterFire = await canvasHash(page, 0, 20, 320, 220);
  assert('Fire touch changes screen (rope fires)', beforeFire !== afterFire);

  // --- Test 11: Multi-touch (d-pad + jump) ---
  console.log('\n10. Testing multi-touch (d-pad left + jump)...');
  const beforeMulti = await canvasHash(page, 0, 20, 320, 220);

  const dpadLeft = canvasToViewport(rect, 0.06, 0.75);
  const jumpMulti = canvasToViewport(rect, 0.73, 0.8);
  await dispatchTouch(cdp, 'touchStart', [dpadLeft, jumpMulti]);
  await sleep(1500);
  await dispatchTouch(cdp, 'touchEnd', []);
  await sleep(500);

  const afterMulti = await canvasHash(page, 0, 20, 320, 220);
  assert('Multi-touch (d-pad + jump) changes screen', beforeMulti !== afterMulti);

  // --- Test 12: Controls fade out after touch ends ---
  console.log('\n11. Testing controls fade out...');
  // Touch to show controls
  await dispatchTouch(cdp, 'touchStart', [centerPt]);
  await sleep(100);
  const duringTouch = await canvasHash(page, 260, 150, 60, 90);
  await dispatchTouch(cdp, 'touchEnd', []);

  // Wait for fade (2s hold + 3s fade = 5s)
  await sleep(6000);
  const afterFade = await canvasHash(page, 260, 150, 60, 90);
  assert('Controls fade out after touch ends', duringTouch !== afterFade,
    `during=${duringTouch}, after=${afterFade}`);

  // --- Test 13: No JS errors ---
  const postGameErrors = pageErrors.filter(e => !e.includes('AudioContext'));
  assert('No JS errors during touch gameplay', postGameErrors.length === 0,
    postGameErrors.length > 0 ? postGameErrors.join('; ') : undefined);

  // --- Test 14: Landscape viewport meta ---
  const viewportMeta = await page.evaluate(() => {
    const meta = document.querySelector('meta[name="viewport"]');
    return meta ? meta.getAttribute('content') : null;
  });
  assert('Viewport meta supports landscape', viewportMeta !== null &&
    viewportMeta.includes('width=device-width'),
    viewportMeta);

  // --- Test 15: Canvas touch-action is none ---
  const touchAction = await page.evaluate(() => {
    const c = document.getElementById('game-canvas');
    return window.getComputedStyle(c).touchAction;
  });
  assert('Canvas has touch-action: none', touchAction === 'none', touchAction);

  // --- Summary ---
  console.log(`\n=== Results: ${passed} passed, ${failed} failed ===`);

  await cdp.detach();
  await browser.close();
  process.exit(failed > 0 ? 1 : 0);
}

run().catch(err => {
  console.error('Test runner error:', err);
  process.exit(1);
});
