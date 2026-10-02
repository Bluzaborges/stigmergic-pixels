cmake_minimum_required(VERSION 3.21)

project(stigmergic-pixels VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)

find_package(SDL2 CONFIG REQUIRED)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC SDL2::SDL2)

set(STIGMERGIC_PIXELS_SOURCES
    main.cpp
    include/App.h
    include/Simulation.h
    src/App.cpp
    src/Simulation.cpp
)

add_executable(stigmergic-pixels ${STIGMERGIC_PIXELS_SOURCES})

if(WIN32)
    set_target_properties(stigmergic-pixels PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES ${STIGMERGIC_PIXELS_SOURCES}
)

target_compile_features(stigmergic-pixels PRIVATE cxx_std_17)
target_include_directories(stigmergic-pixels PRIVATE include)
target_link_libraries(stigmergic-pixels PRIVATE SDL2::SDL2 imgui)

if(MSVC)
    target_compile_options(stigmergic-pixels PRIVATE /W4)
else()
    target_compile_options(stigmergic-pixels PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    add_custom_command(
        TARGET stigmergic-pixels
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:stigmergic-pixels>
        VERBATIM
    )
endif()
