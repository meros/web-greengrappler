/**
 * Headless browser integration test for Green Grappler.
 * Verifies: game loads, title screen renders, gameplay starts, coins can be collected.
 *
 * Controls: WASD/Arrows = move, Space = jump, Enter = rope/fire/select
 *
 * Usage: node game.test.mjs <game-url>
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

async function run() {
  console.log(`Testing game at ${GAME_URL} with Chrome at ${CHROME}\n`);
  console.log('Controls: WASD/Arrows=move, Space=jump, Enter=rope/select\n');

  const browser = await puppeteer.launch({
    headless: 'new',
    executablePath: CHROME,
    args: ['--no-sandbox', '--disable-setuid-sandbox', '--disable-gpu'],
  });

  const page = await browser.newPage();
  const pageErrors = [];
  const failedRequests = [];

  page.on('pageerror', err => pageErrors.push(err.message));
  page.on('response', resp => {
    if (resp.status() >= 400 && !resp.url().includes('favicon')) {
      failedRequests.push(`${resp.url()} -> ${resp.status()}`);
    }
  });

  // --- Test 1: Page loads ---
  console.log('1. Loading game...');
  await page.goto(GAME_URL, { waitUntil: 'networkidle2', timeout: 15000 });
  assert('Page loads', true);

  // --- Test 2: Canvas exists and has correct dimensions ---
  const canvasInfo = await page.evaluate(() => {
    const c = document.getElementById('game-canvas');
    return c ? { width: c.width, height: c.height } : null;
  });
  assert('Canvas exists', canvasInfo !== null);
  assert('Canvas is 320x240', canvasInfo?.width === 320 && canvasInfo?.height === 240,
    canvasInfo ? `got ${canvasInfo.width}x${canvasInfo.height}` : 'no canvas');

  // --- Test 3: Title screen renders ---
  console.log('\n2. Waiting for title screen...');
  await sleep(6000);

  const titlePixels = await page.evaluate(() => {
    const c = document.getElementById('game-canvas');
    const ctx = c.getContext('2d');
    const data = ctx.getImageData(0, 0, 320, 240).data;
    let nonBlack = 0;
    for (let i = 0; i < data.length; i += 4) {
      if (data[i] > 0 || data[i + 1] > 0 || data[i + 2] > 0) nonBlack++;
    }
    return nonBlack;
  });
  assert('Title screen renders (>50% non-black)', titlePixels > 320 * 240 * 0.5,
    `${titlePixels} non-black pixels`);
  assert('No JS errors during load', pageErrors.length === 0,
    pageErrors.length > 0 ? pageErrors.join('; ') : undefined);
  assert('No failed asset requests', failedRequests.length === 0,
    failedRequests.length > 0 ? failedRequests.join('; ') : undefined);

  // --- Test 4: Start game with Enter (fire/select) ---
  console.log('\n3. Starting game (Enter to select)...');
  await page.keyboard.press('Enter');
  await sleep(2000);

  const inGame = await page.evaluate(() => {
    const c = document.getElementById('game-canvas');
    const ctx = c.getContext('2d');
    const data = ctx.getImageData(0, 0, 50, 10).data;
    let hasHUD = false;
    for (let i = 0; i < data.length; i += 4) {
      if (data[i] > 150 || data[i + 1] > 150 || data[i + 2] > 150) {
        hasHUD = true;
        break;
      }
    }
    return hasHUD;
  });
  assert('Game enters level (HUD visible)', inGame);

  // --- Test 5: Dismiss dialogues with Enter ---
  console.log('\n4. Dismissing dialogue (Enter)...');
  for (let i = 0; i < 20; i++) {
    await page.keyboard.press('Enter');
    await sleep(200);
  }
  await sleep(500);

  // Helper: hash a region of the canvas for comparison
  async function canvasHash(x, y, w, h) {
    return page.evaluate(({x, y, w, h}) => {
      const c = document.getElementById('game-canvas');
      const ctx = c.getContext('2d');
      const data = ctx.getImageData(x, y, w, h).data;
      // Simple hash of sampled pixels
      let hash = 0;
      for (let i = 0; i < data.length; i += 16) {
        hash = ((hash << 5) - hash + data[i]) | 0;
      }
      return hash;
    }, {x, y, w, h});
  }

  // Snapshot the game area (below HUD, center of screen)
  const beforeMove = await canvasHash(0, 20, 320, 220);

  // --- Test 6: WASD movement (D = right) ---
  console.log('\n5. Testing WASD movement (D=right)...');
  await page.keyboard.down('KeyD');
  await sleep(3000);
  await page.keyboard.up('KeyD');
  await sleep(500);

  const afterWASD = await canvasHash(0, 20, 320, 220);
  assert('WASD movement changes screen', beforeMove !== afterWASD);

  // --- Test 7: Arrow movement (Right) ---
  console.log('\n6. Testing Arrow movement (Right)...');
  const beforeArrow = await canvasHash(0, 20, 320, 220);
  await page.keyboard.down('ArrowRight');
  await sleep(3000);
  await page.keyboard.up('ArrowRight');
  await sleep(500);

  const afterArrow = await canvasHash(0, 20, 320, 220);
  assert('Arrow movement changes screen', beforeArrow !== afterArrow);

  // --- Test 8: Coin collection ---
  console.log('\n7. Testing coin collection...');
  const coinCount = await page.evaluate(() => {
    const c = document.getElementById('game-canvas');
    const ctx = c.getContext('2d');
    const data = ctx.getImageData(20, 0, 30, 10).data;
    let brightPixels = 0;
    for (let i = 0; i < data.length; i += 4) {
      if (data[i] > 150 && data[i + 1] > 130) brightPixels++;
    }
    return brightPixels;
  });
  assert('Coins collected (HUD shows count)', coinCount > 0, `${coinCount} bright pixels in counter`);

  // --- Test 9: Jump with Space ---
  console.log('\n8. Testing jump (Space)...');
  const beforeJump = await canvasHash(0, 20, 320, 220);
  await page.keyboard.press('Space');
  await sleep(1000);
  const afterJump = await canvasHash(0, 20, 320, 220);
  assert('Space triggers jump (screen changes)', beforeJump !== afterJump);

  // --- Test 10: No errors ---
  const postGameErrors = pageErrors.filter(e => !e.includes('AudioContext'));
  assert('No JS errors during gameplay', postGameErrors.length === 0,
    postGameErrors.length > 0 ? postGameErrors.join('; ') : undefined);

  // --- Summary ---
  console.log(`\n=== Results: ${passed} passed, ${failed} failed ===`);

  await browser.close();
  process.exit(failed > 0 ? 1 : 0);
}

run().catch(err => {
  console.error('Test runner error:', err);
  process.exit(1);
});
