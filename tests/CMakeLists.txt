cmake_minimum_required(VERSION 3.16)

project(SnapsTests)

enable_testing()
add_executable(SnapsTests
        Main.cpp
        BasicTests.cpp
        utils/TestGrid.cpp
        utils/TestGrid.hpp
        utils/TestScene.cpp
        utils/TestScene.hpp
        utils/TestScenePreview.cpp
        utils/TestScenePreview.hpp
)
target_link_libraries(SnapsTests PRIVATE Snaps raylib)
