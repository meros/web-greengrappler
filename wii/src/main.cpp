#include "constants.h"
#include "input.h"
#include "resource.h"
#include "screen.h"
#include "media/sound.h"
#include "media/music.h"
#include "screens/splash_screen.h"
#include "screens/title_screen.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#ifdef HW_RVL
#include <gccore.h>
#include <fat.h>
#include <wiiuse/wpad.h>
#endif

static constexpr int WINDOW_SCALE = 2;

static void preloadAssets() {
    // Images
    const char* images[] = {
        "data/images/tileset1.bmp",
        "data/images/hero_run.bmp", "data/images/hero_jump.bmp",
        "data/images/hero_fall.bmp", "data/images/hero_hurt.bmp",
        "data/images/hook.bmp", "data/images/rope.bmp", "data/images/particles.bmp",
        "data/images/coin.bmp", "data/images/saw.bmp", "data/images/robot.bmp",
        "data/images/checkpoint.bmp", "data/images/debris.bmp",
        "data/images/lavatop.bmp", "data/images/lavafill.bmp",
        "data/images/breakinghooktile.bmp", "data/images/movinghooktile.bmp",
        "data/images/button.bmp", "data/images/door.bmp",
        "data/images/wall.bmp", "data/images/core.bmp",
        "data/images/reactor_shell.bmp", "data/images/boss.bmp",
        "data/images/logo.bmp", "data/images/title.bmp",
        "data/images/hand.bmp", "data/images/font.bmp",
        "data/images/level_select.bmp", "data/images/icons.bmp",
        "data/images/completed_level.bmp",
        "data/images/selected_level_background.bmp",
        "data/images/unselected_level_background.bmp",
        "data/images/darken.bmp", "data/images/level_exit_background.bmp",
        "data/images/dialogue.bmp",
        "data/images/doctor_green_portrait.bmp",
        "data/images/ted_portrait.bmp",
    };
    for (auto& img : images) {
        Resource::preLoad(img);
    }

    // Sounds
    const char* sounds[] = {
        "data/sounds/boot", "data/sounds/start",
        "data/sounds/jump", "data/sounds/land",
        "data/sounds/rope", "data/sounds/hook",
        "data/sounds/no_hook", "data/sounds/hurt",
        "data/sounds/coin", "data/sounds/damage",
        "data/sounds/alarm", "data/sounds/reactor_explosion",
        "data/sounds/boss_saw", "data/sounds/time",
        "data/sounds/timeout", "data/sounds/beep",
        "data/sounds/green_peace", "data/sounds/select",
    };
    for (auto& snd : sounds) {
        Sound::preload(snd);
    }

    // Text files
    const char* texts[] = {
        "data/dialogues/level_select.txt",
        "data/dialogues/boss_unlocked.txt",
        "data/dialogues/1-tutorial1.txt",
        "data/dialogues/2-tutorial2.txt",
    };
    for (auto& txt : texts) {
        Resource::preLoadText(txt);
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::srand(static_cast<unsigned>(std::time(nullptr)));

#ifdef HW_RVL
    fatInitDefault();
    VIDEO_Init();
    WPAD_Init();
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Green Grappler",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // Nearest-neighbor for pixel art

    Resource::init(renderer);
    Sound::init();
    Input::init();

    preloadAssets();

    // Start with splash -> title screen chain
    ScreenManager::add(new TitleScreen());
    ScreenManager::add(new SplashScreen());

    bool running = true;
    Uint64 tickInterval = 1000 / TICKS_PER_SECOND;
    Uint64 lastTick = SDL_GetTicks64();

    while (running && !ScreenManager::isEmpty()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            Input::handleEvent(event);
        }

        if (Input::isPressed(Button::FORCE_QUIT)) {
            running = false;
        }

        Uint64 now = SDL_GetTicks64();
        while (now - lastTick >= tickInterval) {
            Input::update();
            ScreenManager::onLogic();
            Music::update();
            lastTick += tickInterval;

            // Prevent spiral of death
            if (now - lastTick > tickInterval * 5) {
                lastTick = now;
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        ScreenManager::draw(renderer);
        SDL_RenderPresent(renderer);
    }

    ScreenManager::clear();
    Music::shutdown();
    Sound::shutdown();
    Resource::shutdown();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
