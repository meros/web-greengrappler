{
  description = "Green Grappler - A 2D platformer web game";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        assetSrc = ./assets-src;

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

        # Dependencies come straight from web/package-lock.json (importNpmLock
        # fetches each package by the integrity hash in the lockfile), so there
        # is no separate npmDepsHash to keep in sync, e.g. on Dependabot PRs.
        npmLock = {
          npmDeps = pkgs.importNpmLock { npmRoot = ./web; };
          npmConfigHook = pkgs.importNpmLock.npmConfigHook;
        };

        webSrc = ./web;

        typeCheck = pkgs.buildNpmPackage {
          pname = "greengrappler-typecheck";
          version = "1.0.0";
          src = webSrc;
          inherit (npmLock) npmDeps npmConfigHook;
          dontNpmBuild = true;
          buildPhase = ''
            runHook preBuild
            node_modules/.bin/tsc --noEmit
            runHook postBuild
          '';
          installPhase = ''
            mkdir -p $out
            echo "typecheck passed" > $out/result
          '';
        };

        # The bundle is content-hashed (game-<hash>.js) and index.html is
        # rewritten to point at it, so the file can be cached forever.
        webGame = pkgs.buildNpmPackage {
          pname = "greengrappler-web";
          version = "1.0.0";
          src = webSrc;
          inherit (npmLock) npmDeps npmConfigHook;
          dontNpmBuild = true;
          buildPhase = ''
            runHook preBuild
            node_modules/.bin/esbuild src/main.ts \
              --bundle \
              --outdir=out \
              --entry-names='game-[hash]' \
              --format=iife \
              --platform=browser \
              --target=es2020 \
              --minify
            bundle=$(basename out/game-*.js)
            grep -q 'src="game.js"' index.html
            sed "s|src=\"game.js\"|src=\"$bundle\"|" index.html > out/index.html
            runHook postBuild
          '';
          installPhase = ''
            mkdir -p $out
            cp out/game-*.js out/index.html $out/
            ln -s ${convertedAssets} $out/assets
          '';
        };

        # Flat copy with symlinks dereferenced — suitable for upload to GCS, Cloud Run, etc.
        site = pkgs.stdenv.mkDerivation {
          name = "greengrappler-site";
          phases = [ "installPhase" ];
          installPhase = ''
            mkdir -p $out
            cp -rL ${webGame}/* $out/
          '';
        };

        jsParseCheck = pkgs.stdenv.mkDerivation {
          name = "greengrappler-js-parse";
          src = webGame;
          nativeBuildInputs = [ pkgs.nodejs ];
          buildPhase = ''
            for f in $src/game-*.js; do
              node -e "new Function(require('fs').readFileSync('$f','utf-8')); console.log('JS parse OK: $f')"
            done
          '';
          installPhase = ''
            mkdir -p $out
            echo "js-parse passed" > $out/result
          '';
        };

      in {
        checks = {
          inherit typeCheck jsParseCheck;
          build = webGame;
        };

        packages = {
          default = webGame;
          inherit site;
          assets = convertedAssets;
        };

        apps.default = {
          type = "app";
          program = toString (pkgs.writeShellScript "serve-greengrappler" ''
            PORT=''${1:-8080}
            echo "Serving Green Grappler at http://localhost:$PORT"
            ${pkgs.python3}/bin/python3 -m http.server "$PORT" --bind 127.0.0.1 --directory ${webGame}
          '');
        };

        devShells.default = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            nodejs
            esbuild
            typescript
            imagemagick
            xmp
            ffmpeg
            python3
            google-cloud-sdk
          ];
          shellHook = ''
            echo "Green Grappler dev shell"
            echo ""
            echo "  Local:"
            echo "    nix build            Build the game"
            echo "    nix run              Serve locally on :8080"
            echo "    nix flake check      Run type checks"
            echo ""
            echo "  Test (needs Chrome, CHROME_PATH if not on PATH):"
            echo "    nix build .#site -o result && cd web && npm ci && SITE_DIR=../result npm test"
            echo ""
            echo "  Deploys run from GitHub Actions on push to main (see README)."
          '';
        };
      }
    );
}
