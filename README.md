# Green Grappler

A 2D platformer originally made for Speedhack 2011, ported from Java/PlayN to TypeScript + HTML5 Canvas.

**Play it here:** https://green-grappler-309038274515.europe-north2.run.app

## Controls

- **WASD / Arrow keys** — Move
- **Space** — Jump
- **Enter** — Rope / Fire / Select
- **Gamepad** — Fully supported (D-pad, sticks, A/B/X/Y)

## Development

Requires [Nix](https://nixos.org/) with flakes enabled.

```bash
nix build          # Build the game
nix run            # Serve locally on :8080
nix flake check    # Run type checks
```

## Deploy to Cloud Run

```bash
docker build -t gcr.io/PROJECT/greengrappler .
docker push gcr.io/PROJECT/greengrappler
gcloud run deploy greengrappler --image gcr.io/PROJECT/greengrappler --port 8080 --allow-unauthenticated
```

## Credits

- **Programming:** Olof Naessen, Per Larsson, Alexander Schrab
- **Graphics:** Olof Naessen, Timur Kondrakov, Per Larsson
- **Music:** Olof Naessen
