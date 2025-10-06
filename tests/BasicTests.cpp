#include "utils/TestGrid.hpp"
#include "snaps/Grid.hpp"
#include "utils/TestScene.hpp"
#include "utils/TestScenePreview.hpp"

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

void BasicTest() {
    // Prepare test grid 5 x 5
    snaps::Grid grid = MakeTestGrid(5, 5);
    AddSand(grid, 2, 2);

    snaps::SnapsEngine engine(grid);

    TestScene scene(engine, grid);
    scene.Tick(20);

    TestScenePreview preview(scene);
    preview.Show();
}