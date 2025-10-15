cmake_minimum_required(VERSION 3.16)
project(SnapsTests)
enable_testing()

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
        GIT_PROGRESS   TRUE
        GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(googletest)

add_executable(SnapsTests
        Main.cpp
        BasicSceneTests.cpp
        fixtures/SceneTest.cpp
        fixtures/SceneTest.hpp
        utils/TestGrid.cpp
        utils/TestGrid.hpp
        utils/TestScene.cpp
        utils/TestScene.hpp
        utils/TestScenePreview.cpp
        utils/TestScenePreview.hpp
)
target_link_libraries(SnapsTests PRIVATE Snaps raylib gtest gmock)
target_include_directories(SnapsTests SYSTEM PRIVATE external)
target_include_directories(SnapsTests PRIVATE .)

