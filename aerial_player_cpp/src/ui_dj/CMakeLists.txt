# src/ui_dj/CMakeLists.txt

set(AERIAL_QT_DJ_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
)

add_executable(AerialDJ
    main.cpp
    MainWindow.cpp
    MainWindow.h
    DeckWidget.cpp
    DeckWidget.h
    LibraryWidget.cpp
    LibraryWidget.h
    MixerWidget.cpp
    MixerWidget.h
)

target_include_directories(AerialDJ
    PRIVATE
        ${AERIAL_QT_DJ_INCLUDES}
)

target_link_libraries(AerialDJ
    PRIVATE
        aerial_core
        Qt${QT_VERSION_MAJOR}::Widgets
)
