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

# ──────────────────────────────────────────────
# OPTIONAL SQLITE
# ──────────────────────────────────────────────
set(AERIAL_HAVE_SQLITE OFF)

# Try to find vcpkg's sqlite3 port, but don't fail if it's missing
find_package(unofficial-sqlite3 CONFIG QUIET)

if (TARGET unofficial::sqlite3::sqlite3)
    message(STATUS "SQLite found: enabling AERIAL_USE_SQLITE")
    set(AERIAL_HAVE_SQLITE ON)
else()
    message(STATUS "SQLite NOT found: building without SQLite (JSON / no-DB mode).")
endif()

# ──────────────────────────────────────────────
# OPTIONS: which Qt frontends to build
# ──────────────────────────────────────────────
option(BUILD_DESKTOP_UI      "Build Qt desktop player (AerialPlayer)"       ON)
option(BUILD_DJ_UI           "Build Qt DJ UI (AerialDJ)"                    ON)
option(BUILD_VIDEO_EDITOR_UI "Build Qt video editor UI (AerialVideoEditor)" ON)

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
)

if (AERIAL_HAVE_SQLITE)
    target_link_libraries(aerial_core PUBLIC unofficial::sqlite3::sqlite3)
    target_compile_definitions(aerial_core PUBLIC AERIAL_USE_SQLITE=1)
endif()

if (WIN32)
    target_link_libraries(aerial_core PUBLIC Ws2_32)
endif()

# ──────────────────────────────────────────────
# CLI / TERMINAL PLAYER
# ──────────────────────────────────────────────
add_executable(aerial
    src/cli/main.cpp
)

target_link_libraries(aerial PRIVATE aerial_core)
target_include_directories(aerial PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/core)

# ──────────────────────────────────────────────
# QT FRONTENDS (desktop / DJ / editor)
# ──────────────────────────────────────────────
if (BUILD_DESKTOP_UI OR BUILD_DJ_UI OR BUILD_VIDEO_EDITOR_UI)
    # Only look for Qt if we actually build some Qt UI
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
endif()

if (BUILD_DESKTOP_UI)
    add_subdirectory(src/ui_desktop)
endif()

if (BUILD_DJ_UI)
    add_subdirectory(src/ui_dj)
endif()

if (BUILD_VIDEO_EDITOR_UI)
    add_subdirectory(src/ui_videoEditor)
endif()
