# Green Grappler - Wii Homebrew Port

This directory contains the Wii homebrew port of Green Grappler, a 2D platformer with grappling hook mechanics.

## Building

There are multiple ways to build the Wii homebrew binary:

### Method 1: Docker (Recommended) ⭐

The easiest method using the official devkitPro Docker image:

```bash
# From project root
docker build -f Dockerfile.wii -t greengrappler-wii .
docker run --rm -v $(pwd)/wii-output:/output greengrappler-wii \
  bash -c "cp greengrappler.dol /output/boot.dol"
```

Or use the automated build script:

```bash
./build-wii.sh
```

### Method 2: Nix with Docker

Build using Nix, which uses Docker internally:

```bash
# Build source package
nix build .#wii

# Build actual .dol file (requires Docker)
nix build .#wii-docker --impure
```

The `--impure` flag is needed because the build uses Docker.

### Method 3: Native devkitPro

If you have devkitPro installed locally:

```bash
# Install devkitPro (one-time setup)
wget https://github.com/devkitPro/pacman/releases/latest/download/devkitpro-pacman.amd64.deb
sudo dpkg -i devkitpro-pacman.amd64.deb
sudo dkp-pacman -Sy
sudo dkp-pacman -S --noconfirm wii-dev

# Set environment
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC=$DEVKITPRO/devkitPPC
export PATH=$DEVKITPPC/bin:$PATH

# Build
cd wii
make -f Makefile.wii
```

## Installation on Wii

### Prerequisites

- Wii console with Homebrew Channel installed
- SD card (formatted as FAT32)

### Steps

1. Build the game using one of the methods above
2. Locate the built package at `result/apps/greengrappler/` (Nix) or `wii-build/apps/greengrappler/` (Docker script)
3. Copy the entire `apps` directory to the root of your SD card
4. Your SD card structure should look like:
   ```
   SD:/
   └── apps/
       └── greengrappler/
           ├── boot.dol
           ├── meta.xml
           └── data/
               ├── images/
               ├── sounds/
               ├── music/
               ├── rooms/
               └── dialogues/
   ```
5. Insert the SD card into your Wii
6. Launch Homebrew Channel
7. Select "Green Grappler" and press A to launch

## Development

### Project Structure

```
wii/
├── src/              # C++ source code
│   ├── main.cpp      # Entry point
│   ├── entities/     # Game entities
│   ├── media/        # Media handling
│   └── screens/      # Game screens
├── include/          # Header files
├── tests/            # Unit tests
├── CMakeLists.txt    # CMake config (for desktop SDL2 build)
├── Makefile.wii      # devkitPPC Makefile (for Wii build)
└── README.md         # This file
```

### Building Desktop Version

For faster development iteration, you can build the desktop version with SDL2:

```bash
nix build .#desktop
nix run .#desktop
```

Or manually with CMake:

```bash
cd wii
mkdir build && cd build
cmake ..
make
./greengrappler
```

### Cross-Compilation Details

The Wii build targets the PowerPC 750 (Gekko) processor:

- **Toolchain**: devkitPPC (powerpc-eabi-gcc)
- **Architecture**: PowerPC 750, 32-bit big-endian
- **CPU Flags**: `-mrvl -mcpu=750 -meabi -mhard-float`
- **Libraries**: libogc, libfat, libwiiuse
- **Output Format**: DOL (Dolphin Executable)

## Troubleshooting

### Build fails in Docker

- Ensure Docker is installed and running
- Try pulling the image manually: `docker pull devkitpro/devkitppc:latest`
- Check Docker has enough disk space

### Game doesn't appear in Homebrew Channel

- Verify SD card is FAT32 formatted
- Check the directory structure matches the example above
- Ensure `boot.dol` and `meta.xml` are present

### Game crashes on Wii

- Check that all asset files are present in the `data/` directory
- Verify the build completed without errors
- Try rebuilding with debug symbols: add `-g` to CXXFLAGS

## Performance Notes

The Wii has limited resources compared to modern hardware:

- **CPU**: 729 MHz PowerPC 750
- **RAM**: 88 MB (24 MB main + 64 MB additional)
- **GPU**: ATI Hollywood (243 MHz)

Keep assets optimized:
- Use compressed textures where possible
- Keep sound files at reasonable bitrates
- Limit particle effects and complex physics

## References

- [devkitPro Documentation](https://devkitpro.org/wiki/Getting_Started)
- [Wii Homebrew Documentation](https://wiibrew.org/)
- [libogc API Reference](https://libogc.devkitpro.org/)
- [Homebrew Channel](https://wiibrew.org/wiki/Homebrew_Channel)

## License

See the main project README for license information.
