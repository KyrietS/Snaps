cmake_minimum_required(VERSION 3.16)
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

project(Bricks)

set(CMAKE_CXX_STANDARD 23)


# raylib
FetchContent_MakeAvailable(raylib)

# BrickApp
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/**.cpp" "src/**.hpp")
add_executable(BrickApp ${sources})
target_include_directories(BrickApp PRIVATE src)
target_link_libraries(BrickApp PRIVATE raylib)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${SOURCES})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT BrickApp)
