#include "media/font.h"
#include <vector>
#include <cstdio>

BitmapFont::BitmapFont(const std::string& filename, char startChar, char endChar) {
    std::fprintf(stderr, "DEBUG: BitmapFont loading '%s'\n", filename.c_str());
    std::fflush(stderr);

    GameImage glyphImage = Resource::getBitmap(filename);
    if (!glyphImage.texture) {
        std::fprintf(stderr, "DEBUG: BitmapFont - no texture for '%s'\n", filename.c_str());
        std::fflush(stderr);
        return;
    }
    if (!glyphImage.surface) {
        std::fprintf(stderr, "DEBUG: BitmapFont - no surface for '%s', cannot scan glyphs\n", filename.c_str());
        std::fflush(stderr);
        return;
    }

    std::fprintf(stderr, "DEBUG: BitmapFont scanning %dx%d image\n", glyphImage.width, glyphImage.height);
    std::fflush(stderr);

    std::vector<char> chars;
    for (char c = startChar; c <= endChar; c++) {
        chars.push_back(c);
    }

    // Lock the surface once for all pixel reads (much faster than per-pixel lock/unlock)
    SDL_LockSurface(glyphImage.surface);

    auto readPixel = [&](int px, int py) -> uint32_t {
        px += glyphImage.sx;
        py += glyphImage.sy;
        if (px < 0 || py < 0 || px >= glyphImage.surface->w || py >= glyphImage.surface->h) return 0;
        uint8_t* pixels = static_cast<uint8_t*>(glyphImage.surface->pixels);
        int bpp = glyphImage.surface->format->BytesPerPixel;
        uint8_t* p = pixels + py * glyphImage.surface->pitch + px * bpp;
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
        uint8_t r, g, b, a;
        SDL_GetRGBA(pixel, glyphImage.surface->format, &r, &g, &b, &a);
        return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) |
               (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
    };

    uint32_t separatingColor = readPixel(0, 0);
    int scanLine = 0;
    int currGlyphIndex = 0;
    int lastRowHeight = 0;

    while (scanLine < glyphImage.height) {
        int x = 0;
        while (x < glyphImage.width) {
            uint32_t color = readPixel(x, scanLine);
            if (color != separatingColor) {
                int y1 = scanLine;
                while (y1 < glyphImage.height && readPixel(x, y1) != separatingColor) y1++;
                int x1 = x;
                while (x1 < glyphImage.width && readPixel(x1, scanLine) != separatingColor) x1++;

                int w = x1 - x;
                int h = y1 - scanLine;
                lastRowHeight = h;

                char ch = (currGlyphIndex < static_cast<int>(chars.size()))
                    ? chars[currGlyphIndex]
                    : static_cast<char>(200 + currGlyphIndex);
                currGlyphIndex++;
                glyphs_[ch] = glyphImage.subImage(x, scanLine, w, h);
                x = x1;
            }
            x += 1;
        }
        if (lastRowHeight == 0) lastRowHeight = 1; // Prevent infinite loop
        scanLine += lastRowHeight + 1;
    }

    SDL_UnlockSurface(glyphImage.surface);

    if (glyphs_.find('\n') == glyphs_.end() && glyphs_.find(' ') != glyphs_.end()) {
        glyphs_['\n'] = glyphs_[' '];
    }
    fontHeight_ = lastRowHeight;

    std::fprintf(stderr, "DEBUG: BitmapFont loaded %d glyphs, height=%d\n",
        static_cast<int>(glyphs_.size()), fontHeight_);
    std::fflush(stderr);
}

GameImage BitmapFont::getGlyph(char ch) const {
    auto it = glyphs_.find(ch);
    if (it != glyphs_.end()) return it->second;
    auto sp = glyphs_.find(' ');
    if (sp != glyphs_.end()) return sp->second;
    return {};
}

int BitmapFont::getTextWidth(const std::string& text) const {
    int w = 0;
    for (char ch : text) w += getGlyph(ch).width;
    return w;
}

void BitmapFont::draw(SDL_Renderer* renderer, const std::string& text, int x, int y) const {
    int cx = x;
    for (char ch : text) {
        GameImage g = getGlyph(ch);
        g.draw(renderer, cx, y);
        cx += g.width;
    }
}

void BitmapFont::drawCenter(SDL_Renderer* renderer, const std::string& text,
                            int x, int y, int w, int h) const {
    int tx = x + w / 2 - getTextWidth(text) / 2;
    int ty = y + h / 2 - fontHeight_ / 2;
    draw(renderer, text, tx, ty);
}

void BitmapFont::drawWrap(SDL_Renderer* renderer, const std::string& text,
                          int x, int y, int maxWidth, int maxChars) const {
    int cx = x;
    int cy = y;
    int rowHeight = 0;
    int totalChars = 0;

    // Split into words
    std::vector<std::string> words;
    std::string current;
    for (char ch : text) {
        if (ch == ' ') {
            if (!current.empty()) words.push_back(current);
            current.clear();
        } else {
            current += ch;
        }
    }
    if (!current.empty()) words.push_back(current);

    for (size_t wi = 0; wi < words.size(); wi++) {
        std::string word = words[wi];
        if (wi > 0) {
            if (maxWidth > 0 && cx + getTextWidth(word) > maxWidth) {
                cx = x;
                cy += rowHeight;
                rowHeight = 0;
            } else {
                word = " " + word;
            }
        }
        for (char ch : word) {
            if (maxChars >= 0 && totalChars >= maxChars) return;
            GameImage g = getGlyph(ch);
            if (g.height > rowHeight) rowHeight = g.height;
            g.draw(renderer, cx, cy);
            cx += g.width;
            if (ch == '\n') { cx = x; cy += rowHeight; rowHeight = 0; }
            totalChars++;
        }
    }
}
