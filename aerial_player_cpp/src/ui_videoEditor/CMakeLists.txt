# src/ui_videoEditor/CMakeLists.txt

set(AERIAL_QT_EDITOR_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/core
)

add_executable(AerialVideoEditor
    main.cpp
    # later: EditorWindow.cpp / .h, TimelineWidget.cpp / .h, etc.
)

target_include_directories(AerialVideoEditor
    PRIVATE
        ${AERIAL_QT_EDITOR_INCLUDES}
)

target_link_libraries(AerialVideoEditor
    PRIVATE
        aerial_core
        Qt${QT_VERSION_MAJOR}::Widgets
)
