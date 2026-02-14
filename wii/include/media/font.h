#pragma once
#include "resource.h"
#include <string>
#include <map>

class BitmapFont {
public:
    BitmapFont(const std::string& filename, char startChar = ' ', char endChar = 'z');

    void draw(SDL_Renderer* renderer, const std::string& text, int x, int y) const;
    void drawCenter(SDL_Renderer* renderer, const std::string& text,
                    int x, int y, int w, int h) const;
    void drawWrap(SDL_Renderer* renderer, const std::string& text,
                  int x, int y, int maxWidth, int maxChars) const;

private:
    GameImage getGlyph(char ch) const;
    int getTextWidth(const std::string& text) const;

    std::map<char, GameImage> glyphs_;
    int fontHeight_ = 0;
};
