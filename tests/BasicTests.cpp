#include "utils/TestGrid.hpp"
#include "snaps/Grid.hpp"
#include "utils/TestScene.hpp"
#include "utils/TestScenePreview.hpp"

constexpr Color SAND_COLOR = {194, 178, 128, 255};

Snaps::Block SandBlock(const int x, const int y) {
	return Snaps::Block {
		.WorldPosition = {static_cast<float>(x), static_cast<float>(y)},
		.FillColor = SAND_COLOR,
		.IsDynamic = true
	};
}

void AddSand(Snaps::Grid& grid, int x, int y) {
	grid[x, y] = SandBlock(x * Snaps::BOX_SIZE, y * Snaps::BOX_SIZE);
}

void BasicTest() {
	// Prepare test grid 5 x 5
	Snaps::Grid grid = MakeTestGrid(5, 5);
	AddSand(grid, 2, 2);

	Snaps::SnapsEngine engine(grid);

	TestScene scene(engine, grid);
	scene.Tick(20);

	TestScenePreview preview(scene);
	preview.Show();
}