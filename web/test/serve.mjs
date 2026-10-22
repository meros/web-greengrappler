/**
 * Minimal static file server for the tests and local play.
 * Usage: node serve.mjs <dir> [port]   (port 0 = pick a free one)
 */
import { createServer } from 'http';
import { readFile } from 'fs/promises';
import { extname, join, normalize, resolve, sep } from 'path';

const TYPES = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'application/javascript',
  '.png': 'image/png',
  '.mp3': 'audio/mpeg',
  '.ogg': 'audio/ogg',
  '.txt': 'text/plain; charset=utf-8',
};

export function serve(dir, port = 0) {
  const root = resolve(dir);
  const server = createServer(async (req, res) => {
    try {
      const path = decodeURIComponent(new URL(req.url, 'http://x').pathname);
      let file = resolve(join(root, normalize(path)));
      if (file !== root && !file.startsWith(root + sep)) {
        res.writeHead(403).end();
        return;
      }
      if (path.endsWith('/')) file = join(file, 'index.html');
      const body = await readFile(file);
      res.writeHead(200, { 'content-type': TYPES[extname(file)] || 'application/octet-stream' });
      res.end(body);
    } catch {
      res.writeHead(404).end('not found');
    }
  });
  return new Promise(ok => server.listen(port, '127.0.0.1', () => ok(server)));
}

if (import.meta.url === `file://${process.argv[1]}`) {
  const server = await serve(process.argv[2] || '.', Number(process.argv[3] ?? 8080));
  console.log(`Serving on http://127.0.0.1:${server.address().port}`);
}
