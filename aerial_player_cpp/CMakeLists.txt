cmake_minimum_required(VERSION 3.20)
project(aerial_player_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static MSVC runtime for true single-exe (no vcruntime DLLs)
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# --- Tell CMake where the Qt6 config from vcpkg lives ---
set(Qt6_DIR "D:/Code/vcpkg/installed/x64-windows/share/Qt6" CACHE PATH "Qt6 root from vcpkg")
list(APPEND CMAKE_PREFIX_PATH "D:/Code/vcpkg/installed/x64-windows/share/Qt6")

# Enable Qt's automatic moc/uic/rcc globally
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# vcpkg will satisfy these when using the vcpkg toolchain + correct triplet
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

# ──────────────────────────────────────────────
# OPTIONAL SQLITE (auto: if found -> SQLite, else JSON)
# ──────────────────────────────────────────────
set(AERIAL_HAVE_SQLITE OFF)

# Try to find vcpkg's sqlite3 port, but don't fail if it's missing
find_package(unofficial-sqlite3 CONFIG QUIET)

# Optional toggle if you ever want SQLite back
option(AERIAL_USE_SQLITE_BACKEND "Use SQLite instead of JSON for stats DB" OFF)

if (TARGET unofficial::sqlite3::sqlite3 AND AERIAL_USE_SQLITE_BACKEND)
    message(STATUS "SQLite found AND AERIAL_USE_SQLITE_BACKEND=ON: enabling AERIAL_USE_SQLITE")
    set(AERIAL_HAVE_SQLITE ON)
else()
    message(STATUS "Using JSON stats DB (no SQLite backend).")
    set(AERIAL_HAVE_SQLITE OFF)
endif()

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

# If SQLite was found, link it and define AERIAL_USE_SQLITE
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
option(BUILD_DESKTOP_UI      "Build Qt desktop player (AerialPlayer)"       ON)
option(BUILD_DJ_UI           "Build Qt DJ UI (AerialDJ)"                    ON)
option(BUILD_VIDEO_EDITOR_UI "Build Qt video editor UI (AerialVideoEditor)" OFF)

if (BUILD_DESKTOP_UI OR BUILD_DJ_UI OR BUILD_VIDEO_EDITOR_UI)
    # Force Qt6 and include Multimedia (desktop uses it)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)
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
