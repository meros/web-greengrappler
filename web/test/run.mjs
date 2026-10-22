/**
 * Serves a built site and runs the browser tests against it.
 * Usage: node run.mjs [site-dir]   (default: $SITE_DIR, else ./dist)
 * Needs Chrome or Chromium: set CHROME_PATH if it is not in a common location.
 */
import { spawn } from 'child_process';
import { fileURLToPath } from 'url';
import { serve } from './serve.mjs';

const dir = process.argv[2] || process.env.SITE_DIR || 'dist';
const server = await serve(dir);
const url = `http://127.0.0.1:${server.address().port}`;
let failed = false;

for (const test of ['game.test.mjs', 'touch.test.mjs']) {
  console.log(`\n##### ${test}`);
  const code = await new Promise(ok =>
    spawn(process.execPath, [fileURLToPath(new URL(test, import.meta.url)), url], { stdio: 'inherit' })
      .on('exit', c => ok(c ?? 1)));
  if (code !== 0) failed = true;
}

server.close();
process.exit(failed ? 1 : 0);
