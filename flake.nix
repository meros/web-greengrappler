{
  description = "Green Grappler - A 2D platformer (web + Wii homebrew)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

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

        # ── Web build (existing) ───────────────────────────────────
        typeCheck = pkgs.stdenv.mkDerivation {
          name = "greengrappler-typecheck";
          src = ./web;
          nativeBuildInputs = [ pkgs.typescript ];
          buildPhase = ''
            tsc --noEmit
          '';
          installPhase = ''
            mkdir -p $out
            echo "typecheck passed" > $out/result
          '';
        };

        webGame = pkgs.stdenv.mkDerivation {
          name = "greengrappler-web";
          src = ./web;
          nativeBuildInputs = [ pkgs.esbuild ];
          buildPhase = ''
            esbuild src/main.ts \
              --bundle \
              --outfile=game.js \
              --format=iife \
              --platform=browser \
              --target=es2020 \
              --minify
          '';
          installPhase = ''
            mkdir -p $out
            cp game.js $out/
            cp index.html $out/
            ln -s ${convertedAssets} $out/assets
          '';
        };

        site = pkgs.stdenv.mkDerivation {
          name = "greengrappler-site";
          phases = [ "installPhase" ];
          installPhase = ''
            mkdir -p $out
            cp -rL ${webGame}/* $out/
          '';
        };

        nginxConf = pkgs.writeText "nginx.conf" ''
          worker_processes 1;
          error_log /var/log/nginx/error.log warn;
          pid /run/nginx.pid;
          events { worker_connections 512; }
          http {
            include /etc/nginx/mime.types;
            default_type application/octet-stream;
            sendfile on;
            gzip on;
            gzip_types text/html application/javascript text/css audio/ogg;

            server {
              listen 8080;
              root /tmp/site;

              location ~* \.(js|png|mp3|ogg|txt)$ {
                expires 1y;
                add_header Cache-Control "public, immutable";
              }

              location / {
                try_files $uri $uri/ /index.html;
              }
            }
          }
        '';

        dockerImage = pkgs.dockerTools.buildLayeredImage {
          name = "greengrappler";
          tag = "latest";
          contents = [
            pkgs.nginx
            pkgs.fakeNss
          ];
          extraCommands = ''
            mkdir -p tmp/site var/log/nginx var/cache/nginx run etc/nginx
            cp ${webGame}/game.js tmp/site/
            cp ${webGame}/index.html tmp/site/
            cp -rL ${convertedAssets} tmp/site/assets
            cp ${nginxConf} etc/nginx/nginx.conf
            cp ${pkgs.nginx}/conf/mime.types etc/nginx/mime.types
          '';
          config = {
            Cmd = [ "${pkgs.nginx}/bin/nginx" "-g" "daemon off;" ];
            ExposedPorts = { "8080/tcp" = {}; };
          };
        };

        jsParseCheck = pkgs.stdenv.mkDerivation {
          name = "greengrappler-js-parse";
          src = webGame;
          nativeBuildInputs = [ pkgs.nodejs ];
          buildPhase = ''
            node -e "new Function(require('fs').readFileSync('$src/game.js','utf-8')); console.log('JS parse OK')"
          '';
          installPhase = ''
            mkdir -p $out
            echo "js-parse passed" > $out/result
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
        # Builds Wii .dol using Docker and devkitPPC
        # Note: Requires relaxed sandbox - see README for setup
        wiiHomebrew = pkgs.runCommand "greengrappler-wii"
          {
            nativeBuildInputs = [ pkgs.docker ];
            src = ./wii;
            # Allow network and docker access
            __noChroot = true;
            requiredSystemFeatures = [ "big-parallel" ];
          }
          ''
            set -e

            echo ""
            echo "════════════════════════════════════════════════════════"
            echo " Building Green Grappler for Wii Homebrew"
            echo "════════════════════════════════════════════════════════"
            echo " Target: PowerPC 750 (Gekko) @ 729 MHz"
            echo " Toolchain: devkitPPC via Docker"
            echo ""

            # Pull Docker image
            echo "→ Pulling devkitPPC Docker image..."
            docker pull devkitpro/devkitppc:latest

            # Copy source
            cp -r $src/* .
            chmod -R +w .

            # Copy assets
            echo "→ Adding game assets..."
            cp -rL ${wiiAssets}/data .

            # Build
            echo "→ Compiling..."
            docker run --rm \
              -v $(pwd):/project \
              -w /project \
              devkitpro/devkitppc:latest \
              make -j$(nproc)

            # Create output structure
            echo "→ Packaging..."
            mkdir -p $out/apps/greengrappler

            cp greengrappler.dol $out/apps/greengrappler/boot.dol
            cp -rL ${wiiAssets}/data $out/apps/greengrappler/

            # Create meta.xml
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

            # Installation guide
            cat > $out/INSTALL.txt <<'TXT'
Green Grappler - Wii Homebrew
══════════════════════════════════════════════════════════

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

            SIZE=$(du -h $out/apps/greengrappler/boot.dol | cut -f1)

            echo ""
            echo "✓ Build complete!"
            echo "  Binary: boot.dol ($SIZE)"
            echo "  Output: $out/apps/greengrappler/"
            echo ""
          '';

      in {
        checks = {
          inherit typeCheck jsParseCheck;
          build = webGame;
          wii-tests = wiiTests;
          wii-build = wiiDesktop;
        };

        packages = {
          default = webGame;
          inherit site;
          assets = convertedAssets;
          docker = dockerImage;
          desktop = wiiDesktop;
          wii = wiiHomebrew;
          wii-assets = wiiAssets;
        };

        apps = {
          default = {
            type = "app";
            program = toString (pkgs.writeShellScript "serve-greengrappler" ''
              PORT=''${1:-8080}
              echo "Serving Green Grappler at http://localhost:$PORT"
              ${pkgs.python3}/bin/python3 -m http.server "$PORT" --bind 127.0.0.1 --directory ${webGame}
            '');
          };

          desktop = {
            type = "app";
            program = toString (pkgs.writeShellScript "run-greengrappler-desktop" ''
              cd ${wiiDesktop}/share/greengrappler
              exec ${wiiDesktop}/bin/greengrappler
            '');
          };
        };

        devShells.default = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            # Web
            nodejs esbuild typescript
            # Assets
            imagemagick xmp ffmpeg
            # C++ / Desktop
            cmake pkg-config gcc
            SDL2 SDL2_image SDL2_mixer
            # Tools
            python3 docker
          ];
          shellHook = ''
            echo "╔════════════════════════════════════════════════════════╗"
            echo "║       Green Grappler - Development Environment        ║"
            echo "╚════════════════════════════════════════════════════════╝"
            echo ""
            echo "  📦 Web Version:"
            echo "    nix build                Build web game"
            echo "    nix run                  Serve on http://localhost:8080"
            echo "    nix build .#docker       Build Docker image"
            echo ""
            echo "  🖥️  Desktop Version (SDL2):"
            echo "    nix build .#desktop      Build native binary"
            echo "    nix run .#desktop        Build and run"
            echo ""
            echo "  🎮 Wii Homebrew:"
            echo "    nix build .#wii --option sandbox relaxed"
            echo "       Build Wii .dol using Docker + devkitPPC"
            echo ""
            echo "    Alternative (without Nix):"
            echo "      docker build -f Dockerfile.wii ."
            echo "      docker run --rm -v \$(pwd)/wii:/project devkitpro/devkitppc make"
            echo ""
            echo "  🧪 Tests:"
            echo "    nix flake check          Run all checks"
            echo "    nix build .#checks.wii-tests"
            echo ""
            echo "  📚 More info: cat README.md"
          '';
        };

        # Wii development shell with devkitPro tools
        devShells.wii = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            docker
            docker-compose
          ];
          shellHook = ''
            echo "╔════════════════════════════════════════════════════════╗"
            echo "║     Wii Homebrew Development Shell                    ║"
            echo "╚════════════════════════════════════════════════════════╝"
            echo ""
            echo "  Build with Docker (recommended):"
            echo "    docker build -f Dockerfile.wii -t greengrappler-wii ."
            echo "    docker run --rm -v \$(pwd)/wii:/project greengrappler-wii"
            echo ""
            echo "  Or build with Nix (experimental):"
            echo "    nix build .#wii"
            echo ""
            echo "  Output: result/apps/greengrappler/boot.dol"
            echo "  Copy apps/ folder to SD card for Homebrew Channel"
          '';
        };
      }
    );
}
