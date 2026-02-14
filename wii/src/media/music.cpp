#include "media/music.h"
#include <cstdio>

std::string Music::mapPath(const std::string& path) {
    // "data/music/olof12.xm" -> "data/music/olof12.ogg"
    std::string result = path;
    auto pos = result.rfind(".xm");
    if (pos != std::string::npos) {
        result.replace(pos, 3, ".ogg");
    }
    return result;
}

void Music::playSong(const std::string& path) {
    stop();
    std::string file = mapPath(path);
    currentPath_ = path;
    currentMusic_ = Mix_LoadMUS(file.c_str());
    if (!currentMusic_) {
        std::fprintf(stderr, "Failed to load music %s: %s\n", file.c_str(), Mix_GetError());
        return;
    }
    Mix_PlayMusic(currentMusic_, -1);
}

void Music::play() {
    if (currentMusic_) {
        Mix_PlayMusic(currentMusic_, -1);
    }
}

void Music::stop() {
    Mix_HaltMusic();
    if (currentMusic_) {
        Mix_FreeMusic(currentMusic_);
        currentMusic_ = nullptr;
    }
}

void Music::pushSong() {
    stop();
    songStack_.push_back(currentPath_);
}

void Music::popSong() {
    stop();
    if (!songStack_.empty()) {
        std::string path = songStack_.back();
        songStack_.pop_back();
        playSong(path);
    }
}

void Music::update() {
    // No-op: SDL_mixer handles looping
}

void Music::shutdown() {
    stop();
    songStack_.clear();
}
