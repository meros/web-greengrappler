# Nix-based build, then copy into a minimal nginx image for Cloud Run
FROM nixos/nix:latest AS build

COPY . /app
WORKDIR /app

RUN nix build .#site --extra-experimental-features 'nix-command flakes' -o /app/site

# Serve with nginx on :8080 (Cloud Run compatible)
FROM nginx:alpine

RUN rm /etc/nginx/conf.d/default.conf

COPY --from=build /app/site/ /usr/share/nginx/html/

RUN cat > /etc/nginx/conf.d/default.conf <<'EOF'
server {
    listen 8080;
    root /usr/share/nginx/html;

    gzip on;
    gzip_types text/html application/javascript text/css audio/ogg;

    location ~* \.(js|png|mp3|ogg|txt)$ {
        expires 1y;
        add_header Cache-Control "public, immutable";
    }

    location / {
        try_files $uri $uri/ /index.html;
    }
}
EOF

EXPOSE 8080
CMD ["nginx", "-g", "daemon off;"]
