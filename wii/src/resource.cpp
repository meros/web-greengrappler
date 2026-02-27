#include "resource.h"
#include "media/animation.h"
#include "media/font.h"
#include <SDL2/SDL_image.h>
#include <fstream>
#include <sstream>
#include <cstdio>

// GameImage implementation
GameImage::GameImage(SDL_Texture* tex, SDL_Surface* surf, int w, int h)
    : texture(tex), width(w), height(h), sx(0), sy(0), sw(w), sh(h), surface(surf) {}

GameImage::GameImage(SDL_Texture* tex, SDL_Surface* surf, int w, int h, int sx_, int sy_, int sw_, int sh_)
    : texture(tex), width(sw_), height(sh_), sx(sx_), sy(sy_), sw(sw_), sh(sh_), surface(surf) {
    (void)w; (void)h;
}

void GameImage::draw(SDL_Renderer* renderer, int x, int y) const {
    if (!texture) return;
    SDL_Rect src = {sx, sy, sw, sh};
    SDL_Rect dst = {x, y, sw, sh};
    SDL_RenderCopy(renderer, texture, &src, &dst);
}

void GameImage::drawScaled(SDL_Renderer* renderer, int dx, int dy, int dw, int dh,
                           int srcX, int srcY, int srcW, int srcH) const {
    if (!texture) return;
    SDL_Rect src = {sx + srcX, sy + srcY, srcW, srcH};
    SDL_Rect dst = {dx, dy, dw, dh};
    SDL_RenderCopy(renderer, texture, &src, &dst);
}

GameImage GameImage::subImage(int x, int y, int w, int h) const {
    return GameImage(texture, surface, width, height, sx + x, sy + y, w, h);
}

uint32_t GameImage::getPixel(int x, int y) const {
    if (!surface) return 0;
    int px = sx + x;
    int py = sy + y;
    if (px < 0 || py < 0 || px >= surface->w || py >= surface->h) return 0;

    SDL_LockSurface(surface);
    uint8_t* pixels = static_cast<uint8_t*>(surface->pixels);
    int bpp = surface->format->BytesPerPixel;
    uint8_t* p = pixels + py * surface->pitch + px * bpp;

    uint32_t pixel = 0;
    switch (bpp) {
        case 1: pixel = *p; break;
        case 2: pixel = *reinterpret_cast<uint16_t*>(p); break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                pixel = (p[0] << 16) | (p[1] << 8) | p[2];
            else
                pixel = p[0] | (p[1] << 8) | (p[2] << 16);
            break;
        case 4: pixel = *reinterpret_cast<uint32_t*>(p); break;
    }
    SDL_UnlockSurface(surface);

    uint8_t r, g, b, a;
    SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
    return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) |
           (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}

// Resource implementation
std::string Resource::mapImagePath(const std::string& path) {
    std::string result = path;
    // "data/images/X.bmp" -> "data/images/X.png"
    auto pos = result.rfind(".bmp");
    if (pos != std::string::npos) {
        result.replace(pos, 4, ".png");
    }
    return result;
}

std::string Resource::mapTextPath(const std::string& path) {
    return path;
}

void Resource::init(SDL_Renderer* renderer) {
    renderer_ = renderer;
    IMG_Init(IMG_INIT_PNG);
}

void Resource::shutdown() {
    for (auto& [key, entry] : images_) {
        if (entry.texture) SDL_DestroyTexture(entry.texture);
        if (entry.surface) SDL_FreeSurface(entry.surface);
    }
    images_.clear();
    bitmapCache_.clear();

    for (auto& [key, anim] : animationCache_) {
        delete anim;
    }
    animationCache_.clear();

    for (auto& [key, font] : fontCache_) {
        delete font;
    }
    fontCache_.clear();

    texts_.clear();
    IMG_Quit();
}

void Resource::preLoad(const std::string& filename) {
    std::string path = mapImagePath(filename);
    if (images_.count(filename)) return;

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::fprintf(stderr, "Failed to load image %s: %s\n", path.c_str(), IMG_GetError());
        images_[filename] = {};
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    if (!texture) {
        std::fprintf(stderr, "Failed to create texture for %s: %s\n", path.c_str(), SDL_GetError());
        SDL_FreeSurface(surface);
        images_[filename] = {};
        return;
    }

    images_[filename] = {surface, texture};
}

void Resource::preLoadText(const std::string& filename) {
    std::string path = mapTextPath(filename);
    if (texts_.count(filename)) return;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::fprintf(stderr, "Failed to load text %s\n", path.c_str());
        texts_[filename] = "";
        return;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    texts_[filename] = ss.str();
}

bool Resource::isDonePreloading() {
    return true; // Synchronous loading
}

GameImage Resource::getBitmap(const std::string& filename, int tintR, int tintG, int tintB) {
    std::string key = filename + ":" + std::to_string(tintR) + ":" +
                      std::to_string(tintG) + ":" + std::to_string(tintB);
    auto it = bitmapCache_.find(key);
    if (it != bitmapCache_.end()) return it->second;

    auto imgIt = images_.find(filename);
    if (imgIt == images_.end() || !imgIt->second.texture) {
        // Try loading on demand
        preLoad(filename);
        imgIt = images_.find(filename);
        if (imgIt == images_.end() || !imgIt->second.texture) {
            bitmapCache_[key] = {};
            return {};
        }
    }

    auto& entry = imgIt->second;
    int w, h;
    SDL_QueryTexture(entry.texture, nullptr, nullptr, &w, &h);

    if (tintR != 255 || tintG != 255 || tintB != 255) {
        SDL_SetTextureColorMod(entry.texture, tintR, tintG, tintB);
    }

    GameImage img(entry.texture, entry.surface, w, h);
    bitmapCache_[key] = img;
    return img;
}

std::string Resource::getText(const std::string& filename) {
    auto it = texts_.find(filename);
    if (it == texts_.end()) {
        preLoadText(filename);
        it = texts_.find(filename);
        if (it == texts_.end()) return "";
    }
    return it->second;
}

Animation* Resource::getAnimation(const std::string& filename, int numFrames) {
    std::string key = filename + ":" + std::to_string(numFrames);
    auto it = animationCache_.find(key);
    if (it != animationCache_.end()) return it->second;

    auto* anim = new Animation(filename, numFrames);
    animationCache_[key] = anim;
    return anim;
}

BitmapFont* Resource::getFont(const std::string& filename) {
    auto it = fontCache_.find(filename);
    if (it != fontCache_.end()) return it->second;

    auto* font = new BitmapFont(filename);
    fontCache_[filename] = font;
    return font;
}

SDL_Renderer* Resource::getRenderer() {
    return renderer_;
}
