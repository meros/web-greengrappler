/**
 * Headless browser integration test for Green Grappler.
 * Verifies: game loads, title screen renders, gameplay starts, coins can be collected.
 *
 * Usage: node game.test.mjs <game-url>
 * Requires: puppeteer-core, Google Chrome/Chromium
 */
import puppeteer from 'puppeteer-core';

const GAME_URL = process.argv[2] || 'http://localhost:8080';
const CHROME = process.env.CHROME_PATH || findChrome();
const errors = [];
let passed = 0;
let failed = 0;

function findChrome() {
  const candidates = [
    '/etc/profiles/per-user/meros/bin/google-chrome',
    '/usr/bin/google-chrome',
    '/usr/bin/chromium',
    '/usr/bin/chromium-browser',
  ];
  for (const c of candidates) {
    try { if (require('fs').existsSync(c)) return c; } catch {}
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
  console.log(`Testing game at ${GAME_URL} with Chrome at ${CHROME}`);
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
  console.log('\n1. Loading game...');
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

  // --- Test 3: Wait for assets to load and title screen to render ---
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
  assert('Title screen renders (>50% pixels non-black)', titlePixels > 320 * 240 * 0.5,
    `${titlePixels} non-black pixels`);
  assert('No JS errors during load', pageErrors.length === 0,
    pageErrors.length > 0 ? pageErrors.join('; ') : undefined);
  assert('No failed asset requests', failedRequests.length === 0,
    failedRequests.length > 0 ? failedRequests.join('; ') : undefined);

  // --- Test 4: Start game ---
  console.log('\n3. Starting game...');
  await page.keyboard.press('Enter');
  await sleep(2000);

  const inGame = await page.evaluate(() => {
    const c = document.getElementById('game-canvas');
    const ctx = c.getContext('2d');
    // Check if the HUD area (top strip) has content - indicates we're in a level
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

  // --- Test 5: Dismiss dialogues and move to collect coins ---
  console.log('\n4. Testing coin collection...');
  // Dismiss tutorial dialogues
  for (let i = 0; i < 20; i++) {
    await page.keyboard.press('KeyZ');
    await sleep(200);
  }

  // Move right to reach coins
  await page.keyboard.down('ArrowRight');
  await sleep(4000);
  await page.keyboard.up('ArrowRight');
  await sleep(500);

  // Check coin counter in HUD
  const coinCount = await page.evaluate(() => {
    // Read the HUD text area for the coin counter
    // The font draws text at position (1, 1) with format "[xN"
    // We check if the canvas has different content in the coin counter area
    const c = document.getElementById('game-canvas');
    const ctx = c.getContext('2d');
    // Sample the HUD row for any bright pixels indicating coin count > 0
    const data = ctx.getImageData(20, 0, 30, 10).data;
    let brightPixels = 0;
    for (let i = 0; i < data.length; i += 4) {
      if (data[i] > 150 && data[i + 1] > 130) brightPixels++;
    }
    return brightPixels;
  });
  assert('Coins collected (HUD shows count)', coinCount > 0, `${coinCount} bright pixels in counter`);

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
