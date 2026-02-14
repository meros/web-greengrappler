#include "media/font.h"

BitmapFont::BitmapFont(const std::string& filename, char startChar, char endChar) {
    GameImage glyphImage = Resource::getBitmap(filename);
    if (!glyphImage.texture) return;

    std::vector<char> chars;
    for (char c = startChar; c <= endChar; c++) {
        chars.push_back(c);
    }

    uint32_t separatingColor = glyphImage.getPixel(0, 0);
    int scanLine = 0;
    int currGlyphIndex = 0;
    int lastRowHeight = 0;

    while (scanLine < glyphImage.height) {
        int x = 0;
        while (x < glyphImage.width) {
            uint32_t color = glyphImage.getPixel(x, scanLine);
            if (color != separatingColor) {
                int y1 = scanLine;
                while (y1 < glyphImage.height && glyphImage.getPixel(x, y1) != separatingColor) y1++;
                int x1 = x;
                while (x1 < glyphImage.width && glyphImage.getPixel(x1, scanLine) != separatingColor) x1++;

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
        scanLine += lastRowHeight + 1;
    }

    if (glyphs_.find('\n') == glyphs_.end() && glyphs_.find(' ') != glyphs_.end()) {
        glyphs_['\n'] = glyphs_[' '];
    }
    fontHeight_ = lastRowHeight;
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
