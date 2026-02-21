# Green Grappler

A 2D platformer originally made for Speedhack 2011 by Darkbits. Available in multiple versions:

- 🌐 **Web Version** (TypeScript + HTML5 Canvas)
- 🖥️ **Desktop Version** (C++ with SDL2)
- 🎮 **Wii Homebrew** (C++ with libogc)

**Play the web version here:** https://green-grappler-309038274515.europe-north2.run.app

## Controls

### Web & Desktop
- **WASD / Arrow keys** — Move
- **Space** — Jump
- **Enter** — Rope / Fire / Select
- **Gamepad** — Fully supported (D-pad, sticks, A/B/X/Y)

### Wii
- **D-Pad / Nunchuk stick** — Move
- **A Button** — Jump
- **B Button** — Rope / Fire / Select
- **+ Button** — Pause / Select

## Building

Requires [Nix](https://nixos.org/) with flakes enabled.

### Web Version

```bash
nix build           # Build the web game
nix run             # Serve locally on :8080
nix flake check     # Run all checks
```

### Desktop Version (SDL2)

```bash
nix build .#desktop    # Build native binary
nix run .#desktop      # Build and run
```

### Wii Homebrew

**Quick build with Docker:**

```bash
./build-wii.sh
```

**Or with Nix:**

```bash
# Source package
nix build .#wii

# Built .dol (requires Docker)
nix build .#wii-docker --impure
```

See [wii/README.md](wii/README.md) for detailed Wii build instructions and installation guide.

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
