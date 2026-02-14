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
extern "C" {
    #include <gccore.h>
    #include <fat.h>
    #include <wiiuse/wpad.h>
}
#endif

static constexpr int WINDOW_SCALE = 2;

static void preloadAssets() {
    std::fprintf(stderr, "DEBUG: Starting asset preload...\n");
    std::fflush(stderr);

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
    std::fprintf(stderr, "DEBUG: Loading %zu images...\n", sizeof(images) / sizeof(images[0]));
    std::fflush(stderr);
    int image_count = 0;
    for (auto& img : images) {
        Resource::preLoad(img);
        image_count++;
    }
    std::fprintf(stderr, "DEBUG: Loaded %d images\n", image_count);
    std::fflush(stderr);

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
    std::fprintf(stderr, "DEBUG: Loading %zu sounds...\n", sizeof(sounds) / sizeof(sounds[0]));
    std::fflush(stderr);
    int sound_count = 0;
    for (auto& snd : sounds) {
        Sound::preload(snd);
        sound_count++;
    }
    std::fprintf(stderr, "DEBUG: Loaded %d sounds\n", sound_count);
    std::fflush(stderr);

    // Text files
    const char* texts[] = {
        "data/dialogues/level_select.txt",
        "data/dialogues/boss_unlocked.txt",
        "data/dialogues/1-tutorial1.txt",
        "data/dialogues/2-tutorial2.txt",
    };
    std::fprintf(stderr, "DEBUG: Loading %zu text files...\n", sizeof(texts) / sizeof(texts[0]));
    std::fflush(stderr);
    int text_count = 0;
    for (auto& txt : texts) {
        Resource::preLoadText(txt);
        text_count++;
    }
    std::fprintf(stderr, "DEBUG: Loaded %d text files\n", text_count);
    std::fflush(stderr);
}

// C++ main function - called from C wrapper in main_wrapper.c
extern "C" int cpp_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::srand(static_cast<unsigned>(std::time(nullptr)));

#ifdef HW_RVL
    std::fprintf(stderr, "DEBUG: Initializing FAT...\n");
    std::fflush(stderr);
    fatInitDefault();
    std::fprintf(stderr, "DEBUG: FAT initialized\n");
    std::fflush(stderr);

    std::fprintf(stderr, "DEBUG: Initializing VIDEO...\n");
    std::fflush(stderr);
    VIDEO_Init();
    std::fprintf(stderr, "DEBUG: VIDEO initialized\n");
    std::fflush(stderr);

    std::fprintf(stderr, "DEBUG: Initializing WPAD...\n");
    std::fflush(stderr);
    WPAD_Init();
    std::fprintf(stderr, "DEBUG: WPAD initialized\n");
    std::fflush(stderr);
#endif

    std::fprintf(stderr, "DEBUG: Initializing SDL...\n");
    std::fflush(stderr);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        std::fflush(stderr);
        return 1;
    }
    std::fprintf(stderr, "DEBUG: SDL initialized\n");
    std::fflush(stderr);

    std::fprintf(stderr, "DEBUG: Creating window (320x240 * %d = %dx%d)...\n",
        WINDOW_SCALE, SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE);
    std::fflush(stderr);

    // For Wii, fullscreen mode is more reliable
#ifdef HW_RVL
    const int WIDTH = SCREEN_WIDTH;
    const int HEIGHT = SCREEN_HEIGHT;
    const Uint32 FLAGS = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN;
#else
    const int WIDTH = SCREEN_WIDTH * WINDOW_SCALE;
    const int HEIGHT = SCREEN_HEIGHT * WINDOW_SCALE;
    const Uint32 FLAGS = SDL_WINDOW_SHOWN;
#endif

    SDL_Window* window = SDL_CreateWindow(
        "Green Grappler",
        0, 0,
        WIDTH, HEIGHT,
        FLAGS
    );
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        std::fflush(stderr);
        SDL_Quit();
        return 1;
    }
    std::fprintf(stderr, "DEBUG: Window created (%dx%d)\n", WIDTH, HEIGHT);
    std::fflush(stderr);

    std::fprintf(stderr, "DEBUG: Creating renderer...\n");
    std::fflush(stderr);
    // Try hardware accelerated first, fall back to software on Wii
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) {
        std::fprintf(stderr, "DEBUG: Hardware renderer failed, trying software renderer: %s\n", SDL_GetError());
        std::fflush(stderr);
        // Fall back to software rendering
        renderer = SDL_CreateRenderer(window, -1, 0);
    }

    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer failed (both hardware and software): %s\n", SDL_GetError());
        std::fflush(stderr);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    std::fprintf(stderr, "DEBUG: Renderer created\n");
    std::fflush(stderr);

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // Nearest-neighbor for pixel art

    std::fprintf(stderr, "DEBUG: Initializing Resource manager...\n");
    std::fflush(stderr);
    Resource::init(renderer);
    std::fprintf(stderr, "DEBUG: Resource manager initialized\n");
    std::fflush(stderr);

    std::fprintf(stderr, "DEBUG: Initializing Sound...\n");
    std::fflush(stderr);
    Sound::init();
    std::fprintf(stderr, "DEBUG: Sound initialized\n");
    std::fflush(stderr);

    std::fprintf(stderr, "DEBUG: Initializing Input...\n");
    std::fflush(stderr);
    Input::init();
    std::fprintf(stderr, "DEBUG: Input initialized\n");
    std::fflush(stderr);

    std::fprintf(stderr, "DEBUG: Preloading assets...\n");
    std::fflush(stderr);
    preloadAssets();
    std::fprintf(stderr, "DEBUG: Assets preloaded\n");
    std::fflush(stderr);

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
