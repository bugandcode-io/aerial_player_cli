cmake_minimum_required(VERSION 3.20)
project(aerial_player_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static MSVC runtime for true single-exe (no vcruntime DLLs)
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# vcpkg will satisfy these when using the vcpkg toolchain + correct triplet
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

add_executable(aerial
    src/main.cpp
    src/Player.cpp
    src/Playlist.cpp
    src/UI.cpp
	res/aerial.rc
)

# Link against SDL2 and SDL2_mixer imported targets from vcpkg
# SDL2_mixer can be either SDL2_mixer::SDL2_mixer (dynamic)
# or SDL2_mixer::SDL2_mixer-static (static), depending on triplet.
target_link_libraries(aerial PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,
        SDL2_mixer::SDL2_mixer,
        SDL2_mixer::SDL2_mixer-static>
)
