#pragma once
#include <string>
#include <map>
#include <SDL2/SDL.h>

class BitmapFont;
class Animation;

struct GameImage {
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
    // Sub-image region
    int sx = 0;
    int sy = 0;
    int sw = 0;
    int sh = 0;
    // Pixel data for getPixel (loaded on demand)
    SDL_Surface* surface = nullptr;

    GameImage() = default;
    GameImage(SDL_Texture* tex, SDL_Surface* surf, int w, int h);
    GameImage(SDL_Texture* tex, SDL_Surface* surf, int w, int h, int sx_, int sy_, int sw_, int sh_);

    void draw(SDL_Renderer* renderer, int x, int y) const;
    void drawScaled(SDL_Renderer* renderer, int dx, int dy, int dw, int dh,
                    int srcX, int srcY, int srcW, int srcH) const;
    GameImage subImage(int x, int y, int w, int h) const;
    uint32_t getPixel(int x, int y) const;
};

class Resource {
public:
    static void init(SDL_Renderer* renderer);
    static void shutdown();

    static void preLoad(const std::string& filename);
    static void preLoadText(const std::string& filename);
    static bool isDonePreloading();

    static GameImage getBitmap(const std::string& filename, int tintR = 255, int tintG = 255, int tintB = 255);
    static std::string getText(const std::string& filename);
    static Animation* getAnimation(const std::string& filename, int numFrames = -1);
    static BitmapFont* getFont(const std::string& filename);
    static SDL_Renderer* getRenderer();

private:
    static std::string mapImagePath(const std::string& path);
    static std::string mapTextPath(const std::string& path);

    struct ImageEntry {
        SDL_Surface* surface = nullptr;
        SDL_Texture* texture = nullptr;
    };

    static inline SDL_Renderer* renderer_ = nullptr;
    static inline std::map<std::string, ImageEntry> images_;
    static inline std::map<std::string, std::string> texts_;
    static inline std::map<std::string, GameImage> bitmapCache_;
    static inline std::map<std::string, Animation*> animationCache_;
    static inline std::map<std::string, BitmapFont*> fontCache_;
};
