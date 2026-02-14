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
        lib = pkgs.lib;

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

        # ── devkitPro cross-compilation toolchain ──────────────────
        #
        # devkitPPC and its libraries are distributed as pacman .pkg.tar.zst
        # packages from https://pkg.devkitpro.org/packages/linux/x86_64/.
        # We fetch them as fixed-output derivations and unpack into a merged
        # DEVKITPRO sysroot that the Makefile can use directly.
        #
        # To find the current package filenames:
        #   curl -s https://pkg.devkitpro.org/packages/linux/x86_64/ | grep -oE 'href="[^"]*"'
        # Then update the names and hashes below.
        #
        # When a hash is wrong, Nix will tell you the correct one on first build.

        devkitProBaseUrl = "https://pkg.devkitpro.org/packages/linux/x86_64";

        # Helper: fetch a devkitPro pacman package and unpack it
        fetchDkpPkg = { name, hash }: pkgs.stdenv.mkDerivation {
          pname = "dkp-${name}";
          version = "bin";
          src = pkgs.fetchurl {
            url = "${devkitProBaseUrl}/${name}";
            inherit hash;
          };
          nativeBuildInputs = [ pkgs.zstd pkgs.gnutar ];
          sourceRoot = ".";
          unpackPhase = ''
            zstd -d < $src | tar xf -
          '';
          installPhase = ''
            if [ -d opt/devkitpro ]; then
              mkdir -p $out
              cp -r opt/devkitpro/* $out/
            else
              mkdir -p $out
              cp -r . $out/
            fi
          '';
        };

        # Build elf2dol from source (tiny C tool, doesn't need devkitPPC)
        elf2dol = pkgs.stdenv.mkDerivation {
          pname = "elf2dol";
          version = "1.0.5";
          src = pkgs.fetchFromGitHub {
            owner = "devkitPro";
            repo = "general-tools";
            rev = "v1.0.5";
            hash = "sha256-hrsQ7iUJFoCCDmdVYGaFY8y8BQXGG2GAKmqpVElfgRg=";
          };
          buildPhase = ''
            cd elf2dol
            $CC -O2 -o elf2dol elf2dol.c
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp elf2dol/elf2dol $out/bin/
          '';
        };

        #
        # ── Fetch each devkitPro package ──
        #
        # These hashes are placeholders. On first `nix build .#wii`, Nix will
        # fail and print the correct hash for each. Replace them one by one.
        # Alternatively, run: nix-prefetch-url <url>
        #

        dkpDevkitPPC = fetchDkpPkg {
          name = "devkitPPC-r46-1-x86_64.pkg.tar.zst";
          hash = lib.fakeHash;
        };

        dkpLibogc = fetchDkpPkg {
          name = "libogc-2.8.0-1-any.pkg.tar.zst";
          hash = lib.fakeHash;
        };

        dkpWiiSDL2 = fetchDkpPkg {
          name = "wii-sdl2-2.28.5-3-any.pkg.tar.zst";
          hash = lib.fakeHash;
        };

        dkpWiiSDL2Image = fetchDkpPkg {
          name = "wii-sdl2_image-2.8.2-2-any.pkg.tar.zst";
          hash = lib.fakeHash;
        };

        dkpWiiSDL2Mixer = fetchDkpPkg {
          name = "wii-sdl2_mixer-2.8.0-2-any.pkg.tar.zst";
          hash = lib.fakeHash;
        };

        dkpWiiLibpng = fetchDkpPkg {
          name = "wii-libpng-1.6.43-1-any.pkg.tar.zst";
          hash = lib.fakeHash;
        };

        dkpWiiZlib = fetchDkpPkg {
          name = "wii-zlib-1.3.1-2-any.pkg.tar.zst";
          hash = lib.fakeHash;
        };

        dkpWiiLibjpeg = fetchDkpPkg {
          name = "wii-libjpeg-turbo-3.0.3-1-any.pkg.tar.zst";
          hash = lib.fakeHash;
        };

        dkpWiiLibvorbisidec = fetchDkpPkg {
          name = "wii-libvorbisidec-1.2.1-5-any.pkg.tar.zst";
          hash = lib.fakeHash;
        };

        dkpWiiLibogg = fetchDkpPkg {
          name = "wii-libogg-1.3.5-3-any.pkg.tar.zst";
          hash = lib.fakeHash;
        };

        # Merge all devkitPro packages into a single sysroot
        devkitProSysroot = pkgs.symlinkJoin {
          name = "devkitpro-sysroot";
          paths = [
            dkpDevkitPPC
            dkpLibogc
            dkpWiiSDL2
            dkpWiiSDL2Image
            dkpWiiSDL2Mixer
            dkpWiiLibpng
            dkpWiiZlib
            dkpWiiLibjpeg
            dkpWiiLibvorbisidec
            dkpWiiLibogg
          ];
        };

        # ── Wii .dol cross-compilation ─────────────────────────────
        wiiDol = pkgs.stdenv.mkDerivation {
          name = "greengrappler-wii-dol";
          src = ./wii;

          nativeBuildInputs = [ elf2dol pkgs.gnumake ];

          # These env vars are how devkitPro's ecosystem finds the sysroot
          DEVKITPRO = devkitProSysroot;
          DEVKITPPC = "${devkitProSysroot}/devkitPPC";

          buildPhase = ''
            export PATH="$DEVKITPPC/bin:$PATH"

            PREFIX=powerpc-eabi-
            CXX="''${PREFIX}g++"

            MACHDEP="-DGEKKO -mrvl -mcpu=750 -meabi -mhard-float"
            INCLUDE="-I$DEVKITPRO/libogc/include \
                     -I$DEVKITPRO/portlibs/wii/include \
                     -I$DEVKITPRO/portlibs/wii/include/SDL2 \
                     -Iinclude"
            LIBDIRS="-L$DEVKITPRO/libogc/lib/wii \
                     -L$DEVKITPRO/portlibs/wii/lib"
            LIBS="-lSDL2_mixer -lSDL2_image -lSDL2 -lpng -lz \
                  -lvorbisidec -logg -ljpeg -lfat -lwiiuse -lbte -logc -lm"

            CXXFLAGS="-std=c++17 -O2 -Wall $MACHDEP $INCLUDE -DHW_RVL"

            echo "Cross-compiling for Wii (powerpc-eabi) ..."

            # Collect all .cpp source files
            SOURCES=$(find src -name '*.cpp' ! -name 'main.cpp')

            # Compile each source file
            for f in $SOURCES src/main.cpp; do
              obj="''${f%.cpp}.o"
              mkdir -p "$(dirname "$obj")"
              echo "  CXX $f"
              $CXX $CXXFLAGS -c "$f" -o "$obj"
            done

            # Collect all object files
            OBJECTS=$(find src -name '*.o')

            # Link
            echo "  LD greengrappler.elf"
            $CXX $OBJECTS $MACHDEP $LIBDIRS $LIBS -o greengrappler.elf

            # Convert ELF -> DOL (Wii executable format)
            echo "  ELF2DOL greengrappler.dol"
            elf2dol greengrappler.elf greengrappler.dol
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
          '';
        };

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
          wii = wiiDol;
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
            python3
          ];
          shellHook = ''
            echo "Green Grappler dev shell"
            echo ""
            echo "  Web:"
            echo "    nix build                Build web game"
            echo "    nix run                  Serve web game on :8080"
            echo ""
            echo "  Desktop (SDL2):"
            echo "    nix build .#desktop      Build desktop C++ version"
            echo "    nix run .#desktop        Run desktop version"
            echo ""
            echo "  Wii Homebrew:"
            echo "    nix build .#wii          Cross-compile to .dol for Wii HBC"
            echo "    # Output: result/apps/greengrappler/boot.dol"
            echo "    # Copy result/apps/greengrappler/ to SD card"
            echo ""
            echo "  Tests:"
            echo "    nix flake check          Run all checks (web + C++ tests)"
          '';
        };
      }
    );
}
