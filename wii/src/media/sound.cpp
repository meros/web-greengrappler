#include "media/sound.h"
#include <SDL2/SDL.h>
#include <cstdio>

std::string Sound::mapPath(const std::string& path) {
    // "data/sounds/jump" -> "data/sounds/jump.mp3"
    // We keep the original path structure but add extension
    return path + ".mp3";
}

void Sound::init() {
    if (initialized_) return;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::fprintf(stderr, "SDL_mixer init failed: %s\n", Mix_GetError());
        return;
    }
    Mix_AllocateChannels(16);
    initialized_ = true;
}

void Sound::shutdown() {
    for (auto& [path, chunk] : chunks_) {
        if (chunk) Mix_FreeChunk(chunk);
    }
    chunks_.clear();
    if (initialized_) {
        Mix_CloseAudio();
        initialized_ = false;
    }
}

void Sound::preload(const std::string& path) {
    std::string file = mapPath(path);
    if (chunks_.count(file)) return;
    Mix_Chunk* chunk = Mix_LoadWAV(file.c_str());
    if (!chunk) {
        std::fprintf(stderr, "Failed to load sound %s: %s\n", file.c_str(), Mix_GetError());
    }
    chunks_[file] = chunk;
}

void Sound::playSample(const std::string& path) {
    std::string file = mapPath(path);
    auto it = chunks_.find(file);
    if (it == chunks_.end() || !it->second) return;
    Mix_PlayChannel(-1, it->second, 0);
}
