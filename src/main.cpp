#include <vector>
#include <raylib.h>
#include <cmath>
#include <iostream>
#include <raymath.h>
#include <stack>

#include "Block.hpp"
#include "Constants.hpp"
#include "Grid.hpp"

constexpr Color STONE_COLOR = {128, 128, 128, 255};
constexpr Color SAND_COLOR = {194, 178, 128, 255};
constexpr float TIME_MULTIPLIER = .5f;

bool tick = false;

float DeltaTime() {
	// return 0.008f;
	// if (tick) {
	// 	return 0.016f; // simulate 60 FPS
	// }
	// return 0.0f;

	// ONLY FOR DEBUGGING PURPOSES
	float dt =  GetFrameTime() * TIME_MULTIPLIER;
	// if (dt > 0.016f) {
	// 	return 0.016f; // simulate 60 FPS
	// }
	return dt;
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

	grid[11, GetScreenHeight() / BOX_SIZE - 6] = Block {
		.WorldPosition = {11 * BOX_SIZE, (static_cast<float>(GetScreenHeight()) / BOX_SIZE - 6) * BOX_SIZE},
		.FillColor = STONE_COLOR,
		.IsDynamic = false
	};

	for (int gridX = 10; gridX < 20; gridX++) {
		const int gridY = GetScreenHeight() / BOX_SIZE - 20;
		const float worldX = static_cast<float>(gridX) * BOX_SIZE;
		const float worldY = static_cast<float>(gridY) * BOX_SIZE;
		grid[gridX, gridY] = Block {
			.WorldPosition = {worldX, worldY},
			.FillColor = STONE_COLOR,
			.IsDynamic = false
		};
	}
}

void DrawUi(const Grid& grid) {
	DrawFPS(10, GetScreenHeight() - 20);

	for (int y = 0; y < grid.Height(); y++) {
		for (int x = 0; x < grid.Width(); x++) {
			const auto& block = grid[x, y];
			if (block.has_value()) {
				Color color = block->IsDynamic ? BLUE : RED;
				DrawRectangleLines(x * BOX_SIZE, y * BOX_SIZE, BOX_SIZE, BOX_SIZE, color);
			}
		}
	}

	Vector2 gridPos = {GetMousePosition() / BOX_SIZE};
	DrawText(TextFormat(
		"(%d, %d)",
		static_cast<int>(gridPos.x),
		static_cast<int>(gridPos.y)),
		GetScreenWidth() - 50,
		GetScreenHeight() - 20,
		10,
		WHITE);
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

	if (IsKeyReleased(KEY_UP)) {
		for (auto& block : grid.Blocks()) {
			if (block and block->IsDynamic) {
				ApplyImpulse(*block, {0, -400.0f});
			}
		}
	}
	if (IsKeyReleased(KEY_LEFT)) {
		for (auto& block : grid.Blocks()) {
			if (block and block->IsDynamic) {
				ApplyImpulse(*block, {-400.0f, 0});
			}
		}
	}
	if (IsKeyReleased(KEY_RIGHT)) {
		for (auto& block : grid.Blocks()) {
			if (block and block->IsDynamic) {
				ApplyImpulse(*block, {+400.0f, 0});
			}
		}
	}
	if (IsKeyReleased(KEY_SPACE)) {
		tick = true;
		// for (auto& block : grid.Blocks()) {
		// 	if (block and block->IsDynamic) {
		// 		block->Velocity.y = -178.8854;
		// 		// block->Velocity.y = -126.5;
		// 		break;
		// 	}
		// }
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
		block.Velocity.y += GRAVITY * DeltaTime();
	}

	block.WorldPosition.y += block.Velocity.y * DeltaTime();
	if (block.WorldPosition.y >= targetY and below.has_value()) {
		block.WorldPosition.y = targetY;
		if (below->Velocity.y == 0) {
			block.Velocity = {0, 0};
		}
	}
}

struct SecondPassContact {
	int x;
	int y;
};

std::stack<SecondPassContact> rightMovementContacts;
std::stack<SecondPassContact> upMovementContacts;
bool secondPass = false;

void SolveMovementDown(Grid& grid, int& x, int& y, Block& block) {
	// Block is not moving down
	if (block.Velocity.y < 0) return;

	// No block below
	if (not grid.InBounds(x, y+1)) {
		block.Velocity = {0, 0};
		return;
	}

	const int desiredYGrid = static_cast<int>(block.WorldPosition.y + BOX_SIZE) / BOX_SIZE;
	auto& blockBelow = grid[x, y+1];
	const bool wantsToMoveDown = desiredYGrid > y;

	// Desired grid is occupied. Stop.
	if (wantsToMoveDown and blockBelow.has_value()) {
		block.WorldPosition.y = static_cast<float>(y) * BOX_SIZE;
		block.Velocity.y = 0;
		return;
	}

	// [ACCELERATION ONLY]
	// Desired grid is free, claim it.
	if (wantsToMoveDown and not blockBelow.has_value()) {
		blockBelow = block;
		grid.Remove(x, y);
		y += 1;
		return;
	}

	// [ACCELERATION ONLY]
	// Only accelerated movements mid-air collision can result with a stop because
	// the gravity will make the block fall again to the desired spot.
	if (blockBelow and block.WorldPosition.y + BOX_SIZE >= blockBelow->WorldPosition.y) { // collided with block mid-fall
		block.Velocity.y = 0;
	}
}

void SolveMovementUp(Grid& grid, int& x, int& y, Block& block) {
	// Block is not moving up
	if (block.Velocity.y >= 0) return; // moving down

	// No blocks above
	if (not grid.InBounds(x, y-1)) { // at bottom of grid
		block.Velocity = {0, 0};
		return;
	}

	const int desiredYGrid = static_cast<int>(block.WorldPosition.y) / BOX_SIZE;
	auto& blockAbove = grid[x, y - 1];
	const bool wantsToMoveUp = desiredYGrid < y;

	// Desired grid is occupied. Stop.
	if (wantsToMoveUp and blockAbove.has_value()) {
		if (blockAbove->IsDynamic and not secondPass) {
			upMovementContacts.push({x, y});
		} else {
			block.WorldPosition.y = static_cast<float>(y) * BOX_SIZE;
			block.Velocity.y = 0;
		}
		return;
	}

	// [DECELERATION ONLY]
	// Desired grid is free. Claim it if we have enough velocity to reach it.
	if (wantsToMoveUp and not blockAbove.has_value()) {
		const float minVelocityToReachNextGrid = std::sqrtf(2.0f * GRAVITY * BOX_SIZE);

		// Not enough velocity to reach next grid. Stop.
		if (std::abs(block.Velocity.y) < minVelocityToReachNextGrid) {
			block.WorldPosition.y = static_cast<float>(y) * BOX_SIZE;
			block.Velocity.y = 0;
		} else { // Claim grid above.
			blockAbove = block;
			grid.Remove(x, y);
			y -= 1;
		}
	}
}

void SolveMovementLeft(Grid& grid, int& x, int& y, Block& block) {
	// Block is not moving left
	if (block.Velocity.x >= 0) return; // moving right

	// No blocks to the left
	if (not grid.InBounds(x-1, y)) { // at left edge of grid
		block.Velocity = {0, 0};
		return;
	}

	const int desiredXGrid = static_cast<int>(block.WorldPosition.x) / BOX_SIZE;
	auto& blockLeft = grid[x - 1, y];
	const bool wantsToMoveLeft = desiredXGrid < x;

	// Desired grid is occupied. Stop.
	if (wantsToMoveLeft and blockLeft.has_value()) {
		block.WorldPosition.x = static_cast<float>(x) * BOX_SIZE;
		block.Velocity.x = 0;
		return;
	}

	// [DECELERATION ONLY]
	// Desired grid is free. Claim it if we have enough velocity to reach it.
	if (wantsToMoveLeft and not blockLeft.has_value()) {
		// FRICTION here should me maximal deceleration force possible. If it's too low,
		// there might be a possibility that the block will not reach the next grid and be stuck (bugged)
		const float minVelocityToReachNextGrid = std::sqrtf(2.0f * FRICTION * BOX_SIZE);

		// Not enough velocity to reach next grid. Stop.
		if (std::abs(block.Velocity.x) < minVelocityToReachNextGrid) {
			block.WorldPosition.x = static_cast<float>(x) * BOX_SIZE;
			block.Velocity.x = 0;
		} else { // Claim grid to the left.
			blockLeft = block;
			grid.Remove(x, y);
			x -= 1;
		}
	}
}

void SolveMovementRight(Grid& grid, int& x, int& y, Block& block) {
	// Block is not moving right
	if (block.Velocity.x <= 0) return; // moving left

	// No blocks to the right
	if (not grid.InBounds(x+1, y)) { // at right edge of grid
		block.Velocity = {0, 0};
		return;
	}

	const int desiredXGrid = static_cast<int>(block.WorldPosition.x + BOX_SIZE) / BOX_SIZE;
	auto& blockRight = grid[x + 1, y];
	const bool wantsToMoveRight = desiredXGrid > x;

	// Desired grid is occupied. Stop.
	if (wantsToMoveRight and blockRight.has_value()) {
		if (blockRight->IsDynamic and not secondPass) {
			rightMovementContacts.push({x, y});
			x = -999;
		} else {
			block.WorldPosition.x = static_cast<float>(x) * BOX_SIZE;
			block.Velocity.x = 0;
		}
		return;
	}

	// [DECELERATION ONLY]
	// Desired grid is free. Claim it if we have enough velocity to reach it.
	if (wantsToMoveRight and not blockRight.has_value()) {
		const float minVelocityToReachNextGrid = std::sqrtf(2.0f * FRICTION * BOX_SIZE);

		// Not enough velocity to reach next grid. Stop.
		if (std::abs(block.Velocity.x) < minVelocityToReachNextGrid) {
			block.WorldPosition.x = static_cast<float>(x) * BOX_SIZE;
			block.Velocity.x = 0;
		} else { // Claim grid to the right.
			blockRight = block;
			grid.Remove(x, y);
			x += 1;
		}
	}
}

void SolveGridPhysics(Grid& grid, int x, int y) {
	auto& block = grid[x, y];
	if (not block.has_value() or not block->IsDynamic) return;

	if (block->Velocity.x >= 0)
		SolveMovementRight(grid, x, y, *block);
	else
		SolveMovementLeft(grid, x, y, *block);

	if (x == -999) return; // will continue in second pass

	auto& movedBlock = grid[x, y].value();

	if (movedBlock.Velocity.y >= 0)
		SolveMovementDown(grid, x, y, movedBlock);
	else
		SolveMovementUp(grid, x, y, movedBlock);
}

void Integrate(Block& block) {
	if (not block.IsDynamic or block.InvMass <= 0.0f) return;

	Vector2 acceleration{
		block.ForceAccum.x * block.InvMass,
		block.ForceAccum.y * block.InvMass + GRAVITY * block.GravityScale
	};

	block.Velocity += acceleration * DeltaTime();
	block.WorldPosition += block.Velocity * DeltaTime();
	block.ForceAccum = {0, 0};
}

bool TouchesFloor(const Grid& grid, const int x, const int y, Block& block) {
	if (not grid.InBounds(x, y+1)) return true;
	const auto& below = grid[x, y+1];
	return below.has_value() and (block.WorldPosition.y + BOX_SIZE >= below->WorldPosition.y) and below->Velocity.x == 0;
}

void SimulatePhysics(Grid& grid) {
	for (int y = 0; y < grid.Height(); y++) {
		for (int x = 0; x < grid.Width(); x++) {
			auto& block = grid[x, y];
			if (block.has_value() and block->IsDynamic) {
				ApplyGravity(*block);
				if (TouchesFloor(grid, x, y, *block)) {
					ApplyFriction(*block);
				} else {
					ApplyFriction(*block, 0.0f); // drag
				}
				Integrate(*block);
				// SimulateDynamicBlock(grid, x, y, *block);
			}
		}
	}
	// for (int y = 0; y < grid.Height(); y++) {
	// 	for (int x = 0; x < grid.Width(); x++) {
	// 		SolveGridPhysics(grid, x, y);
	// 	}
	// }
	for (int y = grid.Height() - 1; y >= 0; y--) {
		for (int x = 0; x < grid.Width(); x++) {
			SolveGridPhysics(grid, x, y);
		}
		secondPass = true;
		if (rightMovementContacts.size() > 0)
			std::cout << "right seconds pass: " << rightMovementContacts.size() << std::endl;
		while (not rightMovementContacts.empty()) {
			auto [x, y] = rightMovementContacts.top();
			SolveGridPhysics(grid, x, y);
			rightMovementContacts.pop();
		}
		secondPass = false;
		// for (int x = grid.Width() - 1; x >= 0; x--) {
		// 	SolveGridPhysics(grid, x, y);
		// }
	}

	secondPass = true;
	if (upMovementContacts.size() > 0)
		std::cout << "up seconds pass: " << upMovementContacts.size() << std::endl;
	while (not upMovementContacts.empty()) {
		auto [x, y] = upMovementContacts.top();
		SolveGridPhysics(grid, x, y);
		upMovementContacts.pop();
	}

	secondPass = false;
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
	// SetTargetFPS(60);

	Brick::Grid grid(100, 100);

	InitializeMap(grid);

	while (!WindowShouldClose()) // Detect window close button or ESC key
	{
		BeginDrawing(); {
			ClearBackground(BLACK);
			HandleInput(grid);
			SimulatePhysics(grid);
			Draw(grid);
			DrawUi(grid);
			tick = false;
		}
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
