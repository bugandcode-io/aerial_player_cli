# cli/CMakeLists.txt

set(PLATFORM_SOURCES)
if (WIN32)
    list(APPEND PLATFORM_SOURCES res/aerial.rc)
endif()

add_executable(aerial_cli
    main.cpp          # your existing src/main.cpp
    ${PLATFORM_SOURCES}
)

target_link_libraries(aerial_cli
    PRIVATE
        aerial_core   # ← your engine
)

if (WIN32)
    target_include_directories(aerial_cli PRIVATE ../core)
endif()
