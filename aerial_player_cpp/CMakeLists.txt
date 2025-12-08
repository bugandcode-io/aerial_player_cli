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
find_package(unofficial-sqlite3 CONFIG REQUIRED)

# Turn Qt UI on/off from the command line
option(BUILD_QT_UI "Build Qt-based DJ UI" OFF)

# ──────────────────────────────────────────────
# CORE LIBRARY  (everything in src/core)
# ──────────────────────────────────────────────

add_library(aerial_core
    src/core/Player.cpp
    src/core/Playlist.cpp
    src/core/Server.cpp
    src/core/UI.cpp
    src/core/Config.cpp
    src/core/DB.cpp
    src/core/json.hpp
)

target_include_directories(aerial_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

target_link_libraries(aerial_core
    PUBLIC
        SDL2::SDL2
        SDL2::SDL2main
        $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,
            SDL2_mixer::SDL2_mixer,
            SDL2_mixer::SDL2_mixer-static>
        unofficial::sqlite3::sqlite3
)

if (WIN32)
    target_link_libraries(aerial_core PUBLIC Ws2_32)
endif()

# ──────────────────────────────────────────────
# CLI / TERMINAL PLAYER (uses your big main.cpp)
# ──────────────────────────────────────────────

add_executable(aerial
    src/cli/main.cpp
)

target_link_libraries(aerial
    PRIVATE
        aerial_core
)

target_include_directories(aerial
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

# ──────────────────────────────────────────────
# OPTIONAL: QT DJ UI (only if BUILD_QT_UI=ON)
# ──────────────────────────────────────────────

if (BUILD_QT_UI)
    add_subdirectory(src/ui_qt)
endif()
