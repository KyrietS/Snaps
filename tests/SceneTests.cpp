#include "utils/TestGrid.hpp"
#include "snaps/Grid.hpp"
#include "utils/TestScene.hpp"
#include "utils/TestScenePreview.hpp"
#include <gtest/gtest.h>

constexpr Color SAND_COLOR = { 194, 178, 128, 255 };

snaps::Block SandBlock(const int x, const int y) {
    return snaps::Block {
        .WorldPosition = {static_cast<float>(x), static_cast<float>(y)},
        .FillColor = SAND_COLOR,
        .IsDynamic = true
    };
}

void AddSand(snaps::Grid& grid, int x, int y) {
    grid[ x, y ] = SandBlock(x * snaps::BOX_SIZE, y * snaps::BOX_SIZE);
}

TEST(SceneTests, BasicFreeFall) {
    snaps::Grid grid = MakeTestGrid(5, 5);
    AddSand(grid, 2, 2);

    snaps::SnapsEngine engine(grid);

    TestScene scene(engine, grid);

    EXPECT_SCENE(scene, DynamicBlockAt(2, 2));
    EXPECT_SCENE(scene, EmptyBlockAt(2, 3));
    EXPECT_SCENE(scene, StaticBlockAt(2, 4));

    scene.Tick();
    EXPECT_SCENE(scene, EmptyBlockAt(2, 2));
    EXPECT_SCENE(scene, DynamicBlockAt(2, 3));

    scene.TickTime(0.5);
    EXPECT_SCENE(scene, EmptyBlockAt(2, 2));
    EXPECT_SCENE(scene, DynamicBlockAt(2, 3));
    EXPECT_SCENE(scene, BlockAlignedAt(2, 3));
}
