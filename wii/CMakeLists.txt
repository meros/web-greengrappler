cmake_minimum_required(VERSION 3.16)
project(GreenGrappler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sources
set(GAME_SOURCES
    src/camera.cpp
    src/entity.cpp
    src/entity_factory.cpp
    src/game_state.cpp
    src/input.cpp
    src/layer.cpp
    src/resource.cpp
    src/room.cpp
    src/room_loader.cpp
    src/screen.cpp
    src/media/animation.cpp
    src/media/font.cpp
    src/media/music.cpp
    src/media/sound.cpp
    src/entities/boss.cpp
    src/entities/boss_floor.cpp
    src/entities/boss_saw.cpp
    src/entities/boss_wall.cpp
    src/entities/breaking_hook_tile.cpp
    src/entities/button_entity.cpp
    src/entities/coin.cpp
    src/entities/dialogue.cpp
    src/entities/door.cpp
    src/entities/ground_walking_monster.cpp
    src/entities/hero.cpp
    src/entities/lava_sea.cpp
    src/entities/moving_hook_tile.cpp
    src/entities/particle_system.cpp
    src/entities/reactor.cpp
    src/entities/reactor_core.cpp
    src/entities/simple_walking_monster.cpp
    src/entities/spawn_point.cpp
    src/entities/spike.cpp
    src/entities/wall_of_death.cpp
    src/entities/wall_of_death_starter.cpp
    src/screens/end_screen.cpp
    src/screens/level_screen.cpp
    src/screens/level_select_screen.cpp
    src/screens/splash_screen.cpp
    src/screens/title_screen.cpp
)

# Game library (shared between main executable and tests)
add_library(greengrappler_lib STATIC ${GAME_SOURCES})
target_include_directories(greengrappler_lib PUBLIC include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

target_include_directories(greengrappler_lib PUBLIC
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
)
target_link_libraries(greengrappler_lib PUBLIC
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
)

# Main executable
add_executable(greengrappler src/main.cpp)
target_link_libraries(greengrappler PRIVATE greengrappler_lib)

# Tests
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install
install(TARGETS greengrappler RUNTIME DESTINATION bin)
