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

        # Flat copy with symlinks dereferenced — suitable for upload to GCS, Cloud Run, etc.
        site = pkgs.stdenv.mkDerivation {
          name = "greengrappler-site";
          phases = [ "installPhase" ];
          installPhase = ''
            mkdir -p $out
            cp -rL ${webGame}/* $out/
          '';
        };

        # Nginx config for the docker image
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

        # Docker image: nginx serving static files on :8080 (Cloud Run compatible)
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

      in {
        checks = {
          inherit typeCheck jsParseCheck;
          build = webGame;
        };

        packages = {
          default = webGame;
          inherit site;
          assets = convertedAssets;
          docker = dockerImage;
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
            echo "  Deploy to GCP (Cloud Storage):"
            echo "    nix build .#site && gsutil -m rsync -r -d result/ gs://YOUR_BUCKET/"
            echo ""
            echo "  Deploy to GCP (Cloud Run):"
            echo "    nix build .#docker && docker load < result"
            echo "    docker tag greengrappler:latest gcr.io/PROJECT/greengrappler"
            echo "    docker push gcr.io/PROJECT/greengrappler"
            echo "    gcloud run deploy greengrappler --image gcr.io/PROJECT/greengrappler --port 8080 --allow-unauthenticated"
          '';
        };
      }
    );
}
