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

        assetSrc = ./greengrappler/assets/src/main/resources/assets;

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
            # Bundle TypeScript with esbuild
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

      in {
        checks = {
          inherit typeCheck jsParseCheck;
          build = webGame;
        };

        packages = {
          default = webGame;
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
          ];
          shellHook = ''
            echo "Green Grappler dev shell"
            echo "  Build:       nix build"
            echo "  Serve:       nix run"
            echo "  Typecheck:   nix flake check"
            echo "  Test:        nix run && cd web/test && npm install && CHROME_PATH=\$(which google-chrome || which chromium) node game.test.mjs"
          '';
        };
      }
    );
}
