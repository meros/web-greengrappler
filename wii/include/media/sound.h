#pragma once
#include <string>
#include <map>
#include <SDL2/SDL_mixer.h>

class Sound {
public:
    static void init();
    static void shutdown();
    static void preload(const std::string& path);
    static void playSample(const std::string& path);

private:
    static std::string mapPath(const std::string& path);
    static inline std::map<std::string, Mix_Chunk*> chunks_;
    static inline bool initialized_ = false;
};
