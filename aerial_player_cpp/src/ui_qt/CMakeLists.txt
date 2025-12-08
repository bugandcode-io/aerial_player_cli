# src/ui_qt/CMakeLists.txt

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt is only searched when this subdir is added from the root CMake
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(AerialDJ
    main.cpp
    MainWindow.cpp
    MainWindow.h
    DeckWidget.cpp
    DeckWidget.h
    MixerWidget.cpp
    MixerWidget.h
    LibraryWidget.cpp
    LibraryWidget.h
)

target_link_libraries(AerialDJ
    PRIVATE
        Qt6::Widgets
        aerial_core       # reuse your engine
)

target_include_directories(AerialDJ
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}        # src/ui_qt
        ${CMAKE_SOURCE_DIR}/src/core      # engine headers
)
