# Nix-based build, then copy into a minimal nginx image for Cloud Run
FROM nixos/nix:latest AS build

COPY . /app
WORKDIR /app

RUN nix build .#site --extra-experimental-features 'nix-command flakes' -o /app/site

# Serve with nginx on :8080 (Cloud Run compatible)
FROM nginx:alpine

COPY nginx.conf /etc/nginx/conf.d/default.conf
COPY --from=build /app/site/ /usr/share/nginx/html/

EXPOSE 8080
CMD ["nginx", "-g", "daemon off;"]
