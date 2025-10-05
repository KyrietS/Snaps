#include "TestScenePreview.hpp"
#include <raylib.h>

namespace {

std::pair<int, int> ToWindowCoordinates(const Snaps::Block& block) {
	return {static_cast<int>(block.WorldPosition.x), static_cast<int>(block.WorldPosition.y)};
}

void Draw(const Snaps::Grid& grid) {
	const int width = grid.Width();
	const int height = grid.Height();

	// Draw border in the middle of the screen
	const int gridScreenX = (GetScreenWidth() - width * Snaps::BOX_SIZE) / 2;
	const int gridScreenY = (GetScreenHeight() - height * Snaps::BOX_SIZE) / 2;
	const int gridScreenWidth = width * Snaps::BOX_SIZE;
	const int gridScreenHeight = height * Snaps::BOX_SIZE;

	for (const auto& block : grid.Blocks()) {
		if (block.has_value()) {
			auto [x, y] = ToWindowCoordinates(*block);
			DrawRectangle(x + gridScreenX, y + gridScreenY, Snaps::BOX_SIZE, Snaps::BOX_SIZE, block->FillColor);
		}
	}

	// Draw grid outline (don't use DrawRectangleLines because corner is broken)
	DrawLine(gridScreenX, gridScreenY, gridScreenX + gridScreenWidth, gridScreenY, YELLOW);
	DrawLine(gridScreenX, gridScreenY + gridScreenHeight, gridScreenX + gridScreenWidth, gridScreenY + gridScreenHeight, YELLOW);
	DrawLine(gridScreenX, gridScreenY, gridScreenX, gridScreenY + gridScreenHeight, YELLOW);
	DrawLine(gridScreenX + gridScreenWidth, gridScreenY, gridScreenX + gridScreenWidth, gridScreenY + gridScreenHeight, YELLOW);
}
}

TestScenePreview::TestScenePreview(const TestScene &scene) : m_Scene(scene) {}

void TestScenePreview::Show() {
	constexpr int screenWidth = 500;
	constexpr int screenHeight = 500;

	InitWindow(screenWidth, screenHeight, "Snaps Test Scene Preview");
	SetTargetFPS(60);

	while (!WindowShouldClose()) // Detect window close button or ESC key
	{
		BeginDrawing();
		{
			ClearBackground(BLACK);
			Draw(m_Scene.GetCurrentGrid());
		}
		EndDrawing();
	}

	CloseWindow();
}
