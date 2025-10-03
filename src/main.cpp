#include <map>
#include <ranges>
#include <vector>

#include "raylib.h"

constexpr int BOX_SIZE = 16;
constexpr float GRAVITY = 500.0f;

enum class BoxType {
	AIR, STONE, WOOD, SAND, WATER
};

struct Box {
	Vector2 Position = {0, 0};
	Vector2 Velocity = {0, 0};
	BoxType Type = BoxType::AIR;
	Color FillColor = WHITE;
	bool Static = false;
};

// using Pos = std::pair<int, int>;
// std::map<Pos, Box> MAP;
std::array<std::array<Box, 100>, 100> MAP;



BoxType SELECTED_BOX_TYPE = BoxType::SAND;

Color GetColor(BoxType type) {
	switch (type) {
		case BoxType::AIR: return {0, 0, 0, 0};
		case BoxType::STONE: return {128, 128, 128, 255};
		case BoxType::WOOD: return {139, 69, 19, 255};
		case BoxType::SAND: return{194, 178, 128, 255};
		case BoxType::WATER: return {0, 0, 255, 100};
	}
	return WHITE;
}

void SelectTool() {
	if (IsKeyPressed(KEY_ONE)) {
		SELECTED_BOX_TYPE = BoxType::STONE;
	}
	if (IsKeyPressed(KEY_TWO)) {
		SELECTED_BOX_TYPE = BoxType::WOOD;
	}
	if (IsKeyPressed(KEY_THREE)) {
		SELECTED_BOX_TYPE = BoxType::SAND;
	}
	if (IsKeyPressed(KEY_FOUR)) {
		SELECTED_BOX_TYPE = BoxType::WATER;
	}
}

void InitializeMap() {
	for (int gridX = 10; gridX < 50; gridX++) {
		const int gridY = GetScreenHeight() / BOX_SIZE - 5;
		const float worldX = static_cast<float>(gridX) * BOX_SIZE;
		const float worldY = static_cast<float>(gridY) * BOX_SIZE;
		MAP[gridX][gridY] = Box {
			.Position = {worldX, worldY},
			.Type = BoxType::STONE,
			.FillColor = GetColor(BoxType::STONE)
		};
	}
}

void HandleInput() {
	SelectTool();

	const Vector2 mousePos = GetMousePosition();

	// Snap to grid 16x16
	const int worldPosX = static_cast<int>(mousePos.x) / BOX_SIZE * BOX_SIZE;
	const int worldPosY = static_cast<int>(mousePos.y) / BOX_SIZE * BOX_SIZE;

	const int gridPosX = static_cast<int>(mousePos.x) / BOX_SIZE;
	const int gridPosY = static_cast<int>(mousePos.y) / BOX_SIZE;

	DrawRectangleLines(worldPosX, worldPosY, BOX_SIZE, BOX_SIZE, GetColor(SELECTED_BOX_TYPE));

	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		const auto& below = MAP[gridPosX][gridPosY+1];
		if (below.Type != BoxType::AIR and below.Position.y < (gridPosY+1) * BOX_SIZE) return;
		MAP[gridPosX][gridPosY] = Box {
			.Position = {static_cast<float>(worldPosX), static_cast<float>(worldPosY)},
			.Type = SELECTED_BOX_TYPE,
			.FillColor = GetColor(SELECTED_BOX_TYPE)
		};
	}
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		MAP[gridPosX][gridPosY].Type = BoxType::AIR;
	}
}

void SimulateSand(Box& box, size_t x, size_t y) {
	if (y + 1 >= MAP[x].size()) return; // Out of bounds

	Box& below = MAP[x][y + 1];
	const int newGridY = static_cast<int>(box.Position.y + BOX_SIZE) / BOX_SIZE;
	if (below.Type == BoxType::AIR and newGridY != y) {
		// allocate space below
		std::swap(box, below);
		return;
	}

	const int targetY = y * BOX_SIZE;
	if (y < targetY) {
		box.Velocity.y += GRAVITY * GetFrameTime();
	}

	box.Position.y += box.Velocity.y * GetFrameTime();
	if (box.Position.y >= targetY and below.Type != BoxType::AIR) {
		box.Velocity = {0, 0};
		box.Position.y = targetY;
	}

	return;

	const int newY = box.Position.y + box.Velocity.y * GetFrameTime();
	if (newY < targetY) {
		box.Position.y = newY;
	} else if (newY >= targetY) {
		box.Position.y = targetY;
		box.Velocity = {0, 0};
	}
}

void SimulatePhysics() {
	for (size_t x = 0; x < MAP.size(); x++) {
		for (size_t y = 0; y < MAP[x].size(); y++) {
			Box& box = MAP[x][y];
			switch (box.Type) {
				case BoxType::SAND:
					SimulateSand(box, x, y);
					break;
				default:
					break;
			}

			// const int targetY = y * BOX_SIZE;
			// const int newY = box.Position.y + box.Velocity.y * GetFrameTime();
			// if (newY < targetY) {
			// 	box.Position.y = newY;
			// } else if (newY >= targetY) {
			// 	box.Position.y = targetY;
			// 	box.Velocity = {0, 0};
			// }
		}
	}
}

void Draw() {
	for (const auto &row: MAP) {
		for (const auto& box : row ) {
			if (box.Type == BoxType::AIR) continue;
			DrawRectangle(static_cast<int>(box.Position.x), static_cast<int>(box.Position.y), BOX_SIZE, BOX_SIZE, box.FillColor);
		}
	}
}

void DrawUi() {
	// Draw the selected tool
	const char *toolName = "";
	switch (SELECTED_BOX_TYPE) {
		case BoxType::AIR: toolName = "Air"; break;
		case BoxType::STONE: toolName = "Stone"; break;
		case BoxType::WOOD: toolName = "Wood"; break;
		case BoxType::SAND: toolName = "Sand"; break;
		case BoxType::WATER: toolName = "Water"; break;
	}
	DrawText(TextFormat("Selected Tool: %s", toolName), 10, 10, 20, WHITE);
	DrawText("Press 1-4 to select tool", 10, 30, 10, WHITE);

	DrawFPS(10, GetScreenHeight() - 20);
}


int main() {
	constexpr int screenWidth = 800;
	constexpr int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "Simple Raylib Window - Brick App");
	// SetTargetFPS(120);

	InitializeMap();

	while (!WindowShouldClose()) // Detect window close button or ESC key
	{
		BeginDrawing(); {
			ClearBackground(BLACK);
			HandleInput();
			SimulatePhysics();
			Draw();
			DrawUi();
		}
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
