#pragma once
#include <string>

struct LevelDescription {
    std::string name;
    std::string levelFile;
    int frameIndex;
    std::string musicFile;

    LevelDescription() : frameIndex(0) {}
    LevelDescription(const std::string& n, const std::string& lf, int fi, const std::string& mf)
        : name(n), levelFile(lf), frameIndex(fi), musicFile(mf) {}
};
