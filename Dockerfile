# Nix-based build, then copy into a minimal nginx image for Cloud Run.
# Base images are pinned to an exact tag and digest. Bump them on purpose
# (Dependabot opens PRs for the docker ecosystem as well).

# nixos/nix 2.35.3
FROM nixos/nix:2.35.4@sha256:f159a58f390677f41e8e900a74b6e2f9c7f27b364122ae6acf7a652ce6c551c0 AS build

COPY . /app
WORKDIR /app

RUN nix build .#site --extra-experimental-features 'nix-command flakes' -o /app/site

# nginx 1.30.6 (stable line) on Alpine 3.24
FROM nginx:1.31.8-alpine3.24@sha256:785835ea65db4bba6f021acc0f3be91b0fef76a53fe41f8588c6698d2aed0606

# Replaces the image's main config; the stock conf.d/default.conf is removed so
# only our server block exists. Writes only to /tmp, so it can run as non-root.
RUN rm -f /etc/nginx/conf.d/default.conf
COPY nginx.conf /etc/nginx/nginx.conf
COPY --from=build /app/site/ /usr/share/nginx/html/

# Files from the Nix store all carry mtime 1970-01-01, and nginx builds the
# ETag from mtime and size. A new index.html has the same size (the bundle hash
# has a fixed width), so browsers would get a 304 for a stale page forever.
# Stamping the build time makes Last-Modified and ETag change on every deploy.
RUN find /usr/share/nginx/html -exec touch -h {} +

USER nginx

# Fail the build on a broken config instead of at deploy time. Run as the
# runtime user so it leaves no root-owned files in /tmp.
RUN nginx -t && rm -rf /tmp/nginx.pid

EXPOSE 8080
CMD ["nginx", "-g", "daemon off;"]
