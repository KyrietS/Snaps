cmake_minimum_required(VERSION 3.16)
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_PROGRESS   TRUE
    GIT_SHALLOW    TRUE
)

project(Snaps)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# raylib
FetchContent_MakeAvailable(raylib)

# SnapsApp
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/**.cpp" "src/**.hpp")
add_library(Snaps STATIC ${sources})
target_include_directories(Snaps PRIVATE src PUBLIC include)
target_link_libraries(Snaps PRIVATE raylib) # TODO Remove this later

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${SOURCES})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SnapsSandbox)

add_subdirectory(sandbox)
add_subdirectory(tests)