#include <map>
#include <ranges>
#include <vector>
#include <raylib.h>

#include "Block.hpp"
#include "Grid.hpp"

constexpr int BOX_SIZE = 16;
constexpr float GRAVITY = 500.0f;

constexpr Color STONE_COLOR = {128, 128, 128, 255};
constexpr Color SAND_COLOR = {194, 178, 128, 255};


void DrawUi() {
	DrawFPS(10, GetScreenHeight() - 20);
}

namespace Brick {
void InitializeMap(Grid& grid) {
	for (int gridX = 10; gridX < 50; gridX++) {
		const int gridY = GetScreenHeight() / BOX_SIZE - 5;
		const float worldX = static_cast<float>(gridX) * BOX_SIZE;
		const float worldY = static_cast<float>(gridY) * BOX_SIZE;
		grid[gridX, gridY] = Block {
			.WorldPosition = {worldX, worldY},
			.FillColor = STONE_COLOR,
			.IsDynamic = false
		};
	}
}

void HandleInput(Grid& grid) {
	const Vector2 mousePos = GetMousePosition();

	// Snap to grid 16x16
	const int worldPosX = static_cast<int>(mousePos.x) / BOX_SIZE * BOX_SIZE;
	const int worldPosY = static_cast<int>(mousePos.y) / BOX_SIZE * BOX_SIZE;

	const int gridPosX = static_cast<int>(mousePos.x) / BOX_SIZE;
	const int gridPosY = static_cast<int>(mousePos.y) / BOX_SIZE;

	DrawRectangleLines(worldPosX, worldPosY, BOX_SIZE, BOX_SIZE, SAND_COLOR);

	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		const auto& below = grid[gridPosX, gridPosY+1];
		if (below.has_value() and below->WorldPosition.y < (gridPosY+1) * BOX_SIZE) return;
		grid[gridPosX, gridPosY] = Block {
			.WorldPosition = {static_cast<float>(worldPosX), static_cast<float>(worldPosY)},
			.FillColor = SAND_COLOR,
			.IsDynamic = true
		};
	}
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		grid[gridPosX, gridPosY] = std::nullopt;
	}
}

void SimulateDynamicBlock(Grid& grid, const int x, const int y, Block& block) {
	if (not grid.InBounds(x, y+1)) {
		block.Velocity.y = 0;
		return;
	}

	auto& below = grid[x, y+1];
	const int newGridY = static_cast<int>(block.WorldPosition.y + BOX_SIZE) / BOX_SIZE;
	if (not below.has_value() and newGridY != y) {
		below = block;
		grid.Remove(x, y);
		return;
	}

	const int targetY = y * BOX_SIZE;
	if (y < targetY) {
		block.Velocity.y += GRAVITY * GetFrameTime();
	}

	block.WorldPosition.y += block.Velocity.y * GetFrameTime();
	if (block.WorldPosition.y >= targetY and below.has_value()) {
		block.WorldPosition.y = targetY;
		if (below->Velocity.y == 0) {
			block.Velocity = {0, 0};
		}
	}
}

void SimulatePhysics(Grid& grid) {
	for (int y = 0; y < grid.Height(); y++) {
		for (int x = 0; x < grid.Width(); x++) {
			auto& block = grid[x, y];
			if (block.has_value() and block->IsDynamic) {
				SimulateDynamicBlock(grid, x, y, *block);
			}
		}
	}
}

void Draw(const Grid& grid) {
	for (const auto& block : grid.Blocks()) {
		if (block.has_value()) {
			DrawRectangle(static_cast<int>(block->WorldPosition.x), static_cast<int>(block->WorldPosition.y), BOX_SIZE, BOX_SIZE, block->FillColor);
		}
	}
}
}


int main() {
	constexpr int screenWidth = 800;
	constexpr int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "Simple Raylib Window - Brick App");

	Brick::Grid grid(100, 100);

	InitializeMap(grid);

	while (!WindowShouldClose()) // Detect window close button or ESC key
	{
		BeginDrawing(); {
			ClearBackground(BLACK);
			HandleInput(grid);
			SimulatePhysics(grid);
			Draw(grid);
			DrawUi();
		}
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
