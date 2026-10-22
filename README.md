# Green Grappler

A 2D platformer originally made for Speedhack 2011, ported from Java/PlayN to TypeScript + HTML5 Canvas.

**Play it here:** https://grappler.meros.vip

## Controls

- **WASD / Arrow keys** — Move
- **Space** — Jump
- **Enter** — Rope / Fire / Select
- **Gamepad** — Fully supported (D-pad, sticks, A/B/X/Y)
- **Touch** — On-screen buttons on phones and tablets

## Development

Requires [Nix](https://nixos.org/) with flakes enabled. Dependencies are pinned
by `flake.lock` (nixpkgs) and `web/package-lock.json` (npm).

```bash
nix build .#site   # Build the site (bundle, index.html, converted assets)
nix run            # Serve locally on :8080
nix flake check    # Typecheck, bundle parse check and build
```

The bundle is built as `game-<content hash>.js` and `index.html` is rewritten to
point at it, so the bundle can be cached forever while `index.html` is always
revalidated.

### Tests

Browser tests drive a headless Chrome with puppeteer (title screen, keyboard and
touch controls, coin pickup, jump). They need Chrome or Chromium; set
`CHROME_PATH` if it is not in a common location.

```bash
nix build .#site -o site
cd web && npm ci
CHROME_PATH=$(which google-chrome) SITE_DIR=../site npm test
```

CI runs the same steps (`.github/workflows/check.yml`) on every pull request,
and before every deploy.

### Updating dependencies

Dependabot opens weekly grouped PRs for npm, GitHub Actions and the Docker base
images. Nix reads `web/package-lock.json` directly, so a lockfile change needs
no hash update. `nix flake update` moves nixpkgs.

## Deploy

A push to `main` runs the checks and then deploys to Cloud Run through the
shared workflow in `meros/deploy-workflows` (see `.github/workflows/deploy.yml`).
The image is built by `Dockerfile`: Nix builds the site, nginx serves it on
port 8080 as a non-root user. `nginx.conf` sets the security headers
(including a Content-Security-Policy), the cache rules and a `/health`
endpoint. Cloud Run reserves paths ending in `z`, so it is not `/healthz`.

To build and try the image locally:

```bash
docker build -t greengrappler .
docker run --rm -p 8080:8080 greengrappler
```

## Credits

- **Programming:** Olof Naessen, Per Larsson, Alexander Schrab
- **Graphics:** Olof Naessen, Timur Kondrakov, Per Larsson
- **Music:** Olof Naessen

## Status

Playable in the browser, including on phones. Personal project, updated now and then.

## License

No license is granted. The code, graphics and music belong to their authors listed above.
