set(AERIAL_QT_DESKTOP_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
)

set(AERIAL_QT_DESKTOP_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
)

add_executable(AerialPlayer
    ${AERIAL_QT_DESKTOP_SOURCES}
)

target_include_directories(AerialPlayer
    PRIVATE
        ${AERIAL_QT_DESKTOP_INCLUDES}
)

target_link_libraries(AerialPlayer
    PRIVATE
        aerial_core
        Qt6::Widgets
        Qt6::Multimedia
)

set_target_properties(AerialPlayer PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)
