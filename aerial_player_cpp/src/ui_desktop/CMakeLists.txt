# src/ui_desktop/CMakeLists.txt

set(AERIAL_QT_DESKTOP_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
)

add_executable(AerialPlayer
    main.cpp          # your desktop main (will take argv[1])
    # add more files as you build the UI, e.g.:
    # MainWindow.cpp
    # MainWindow.h
)

target_include_directories(AerialPlayer
    PRIVATE
        ${AERIAL_QT_DESKTOP_INCLUDES}
)

target_link_libraries(AerialPlayer
    PRIVATE
        aerial_core
        Qt${QT_VERSION_MAJOR}::Widgets
)
