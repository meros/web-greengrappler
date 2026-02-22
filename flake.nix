{
  description = "Green Grappler - A 2D platformer (Wii homebrew)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    devkitNix.url = "github:bandithedoge/devkitNix";
  };

  outputs = { self, nixpkgs, flake-utils, devkitNix }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ devkitNix.overlays.default ];
        };

        assetSrc = ./assets-src;

        # ── Shared asset pipeline ──────────────────────────────────
        convertedAssets = pkgs.stdenv.mkDerivation {
          name = "greengrappler-assets";
          src = assetSrc;
          nativeBuildInputs = with pkgs; [ imagemagick xmp ffmpeg ];
          phases = [ "buildPhase" "installPhase" ];
          buildPhase = ''
            mkdir -p build/images build/sounds build/music build/rooms build/dialogues

            for f in $src/data/images/*.bmp; do
              name=$(basename "$f" .bmp)
              convert "$f" -transparent "#FF00FF" "build/images/$name.png"
            done

            for f in $src/data/images/*.png; do
              [ -f "$f" ] && cp "$f" build/images/
            done

            cp $src/data/sounds/*.mp3 build/sounds/

            for f in $src/data/music/*.xm; do
              name=$(basename "$f" .xm)
              xmp -d wav -o "build/music/$name.wav" "$f" 2>/dev/null || true
              if [ -f "build/music/$name.wav" ]; then
                ffmpeg -y -i "build/music/$name.wav" -c:a libvorbis -q:a 5 "build/music/$name.ogg" 2>/dev/null || true
                rm -f "build/music/$name.wav"
              fi
            done

            cp $src/data/rooms/*.txt build/rooms/
            cp $src/data/dialogues/*.txt build/dialogues/
          '';
          installPhase = ''
            mkdir -p $out
            cp -r build/* $out/
          '';
        };

        # Assets laid out for the Wii/desktop C++ build (data/ prefix paths)
        wiiAssets = pkgs.stdenv.mkDerivation {
          name = "greengrappler-wii-assets";
          phases = [ "installPhase" ];
          installPhase = ''
            mkdir -p $out/data/images $out/data/sounds $out/data/music $out/data/rooms $out/data/dialogues
            cp ${convertedAssets}/images/* $out/data/images/
            cp ${convertedAssets}/sounds/* $out/data/sounds/
            cp ${convertedAssets}/music/* $out/data/music/ 2>/dev/null || true
            cp ${convertedAssets}/rooms/* $out/data/rooms/
            cp ${convertedAssets}/dialogues/* $out/data/dialogues/
          '';
        };

        # ── Wii / Desktop C++ build ───────────────────────────────
        wiiDesktop = pkgs.stdenv.mkDerivation {
          name = "greengrappler-desktop";
          src = ./wii;
          nativeBuildInputs = with pkgs; [ cmake pkg-config ];
          buildInputs = with pkgs; [ SDL2 SDL2_image SDL2_mixer ];
          cmakeFlags = [ "-DBUILD_TESTS=ON" ];
          buildPhase = ''
            cmake --build . --parallel
          '';
          installPhase = ''
            mkdir -p $out/bin $out/share/greengrappler
            cp greengrappler $out/bin/
            cp -rL ${wiiAssets}/data $out/share/greengrappler/
          '';
        };

        # Unit tests for the C++ port (no SDL2 needed for pure logic tests)
        wiiTests = pkgs.stdenv.mkDerivation {
          name = "greengrappler-wii-tests";
          src = ./wii;
          nativeBuildInputs = with pkgs; [ cmake pkg-config ];
          buildInputs = with pkgs; [ SDL2 SDL2_image SDL2_mixer ];
          cmakeFlags = [ "-DBUILD_TESTS=ON" ];
          buildPhase = ''
            cmake --build . --parallel
          '';
          doCheck = true;
          checkPhase = ''
            ctest --output-on-failure
          '';
          installPhase = ''
            mkdir -p $out
            echo "all tests passed" > $out/result
          '';
        };

        # ── Wii Homebrew Build ─────────────────────────────────────
        # Builds Wii .dol using devkitNix (native Nix cross-compilation)
        wiiHomebrew = pkgs.devkitNix.stdenvPPC.mkDerivation {
          name = "greengrappler-wii";
          src = ./wii;

          buildPhase = ''
            cp -rL ${wiiAssets}/data .
            make -j$NIX_BUILD_CORES
          '';

          installPhase = ''
            mkdir -p $out/apps/greengrappler

            cp greengrappler.dol $out/apps/greengrappler/boot.dol
            cp -rL ${wiiAssets}/data $out/apps/greengrappler/

            cat > $out/apps/greengrappler/meta.xml <<'XML'
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<app version="1">
  <name>Green Grappler</name>
  <coder>Darkbits</coder>
  <version>1.0.0</version>
  <release_date>20260214</release_date>
  <short_description>2D Grappling Hook Platformer</short_description>
  <long_description>Green Grappler is a 2D platformer with grappling hook mechanics. Originally made for Speedhack 2011 by Darkbits. Ported to Wii homebrew.</long_description>
</app>
XML

            cat > $out/INSTALL.txt <<'TXT'
Green Grappler - Wii Homebrew

Installation:
1. Copy apps/ folder to SD card root
2. Insert SD card into Wii
3. Launch Homebrew Channel
4. Select "Green Grappler"

SD Card Structure:
  SD:/apps/greengrappler/boot.dol
  SD:/apps/greengrappler/meta.xml
  SD:/apps/greengrappler/data/...

Controls:
  D-Pad/Analog: Move
  A: Jump
  B: Grappling Hook
  HOME: Exit
TXT
          '';
        };

      in {
        checks = {
          wii-tests = wiiTests;
          wii-build = wiiDesktop;
        };

        packages = {
          default = wiiDesktop;
          desktop = wiiDesktop;
          wii = wiiHomebrew;
          wii-assets = wiiAssets;
          assets = convertedAssets;
        };

        apps = {
          default = {
            type = "app";
            program = toString (pkgs.writeShellScript "run-greengrappler-desktop" ''
              cd ${wiiDesktop}/share/greengrappler
              exec ${wiiDesktop}/bin/greengrappler
            '');
          };

          dolphin = {
            type = "app";
            program = toString (pkgs.writeShellScript "run-greengrappler-dolphin" ''
              SD_DIR="$HOME/.local/share/dolphin-emu/Load/WiiSDSync/apps/greengrappler"

              # Deploy boot.dol and assets from Nix build output
              mkdir -p "$SD_DIR/data"
              cp -f ${wiiHomebrew}/apps/greengrappler/boot.dol "$SD_DIR/boot.dol"
              cp -rLf ${wiiHomebrew}/apps/greengrappler/data/* "$SD_DIR/data/"

              exec ${pkgs.dolphin-emu}/bin/dolphin-emu \
                --config 'GFX.Settings.AspectRatio=1' \
                -e "$SD_DIR/boot.dol"
            '');
          };
        };

        devShells = {
          default = pkgs.mkShell {
            nativeBuildInputs = with pkgs; [
              # C++ / Desktop
              cmake pkg-config gcc
              SDL2 SDL2_image SDL2_mixer
              # Assets
              imagemagick xmp ffmpeg
              # Tools
              dolphin-emu
            ];
            shellHook = ''
              echo "Green Grappler - Development Environment"
              echo ""
              echo "  Desktop (SDL2):"
              echo "    nix build              Build native binary"
              echo "    nix run                Build and run"
              echo ""
              echo "  Wii Homebrew:"
              echo "    nix build .#wii"
              echo ""
              echo "  Tests:"
              echo "    nix flake check        Run all checks"
            '';
          };

          wii = (pkgs.mkShell.override { stdenv = pkgs.devkitNix.stdenvPPC; }) {
            shellHook = ''
              echo "Green Grappler - Wii Dev Shell (devkitPPC)"
              echo "  DEVKITPRO=$DEVKITPRO"
              echo "  DEVKITPPC=$DEVKITPPC"
              echo ""
              echo "  cd wii && make"
            '';
          };
        };
      }
    );
}
