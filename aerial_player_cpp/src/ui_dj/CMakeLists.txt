set(AERIAL_QT_DJ_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
)

set(AERIAL_QT_DJ_SOURCES
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

add_executable(AerialDJ
    ${AERIAL_QT_DJ_SOURCES}
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

set_target_properties(AerialDJ PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)
