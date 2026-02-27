find_package(PkgConfig REQUIRED)

# Lightweight test runner — no external framework needed
add_executable(test_vec2 test_vec2.cpp)
target_include_directories(test_vec2 PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME test_vec2 COMMAND test_vec2)

add_executable(test_collision test_collision.cpp)
target_include_directories(test_collision PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME test_collision COMMAND test_collision)

add_executable(test_utils test_utils.cpp)
target_include_directories(test_utils PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME test_utils COMMAND test_utils)

add_executable(test_player_skill test_player_skill.cpp)
target_include_directories(test_player_skill PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME test_player_skill COMMAND test_player_skill)

add_executable(test_game_state test_game_state.cpp)
target_include_directories(test_game_state PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_game_state PRIVATE greengrappler_lib)
add_test(NAME test_game_state COMMAND test_game_state)

add_executable(test_level_description test_level_description.cpp)
target_include_directories(test_level_description PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME test_level_description COMMAND test_level_description)
