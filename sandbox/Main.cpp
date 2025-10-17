#include <vector>
#include <raylib.h>
#include <cmath>
#include <iostream>
#include <raymath.h>
#include <stack>

#include "snaps/Block.hpp"
#include "snaps/Constants.hpp"
#include "snaps/Grid.hpp"
#include "snaps/SnapsEngine.hpp"

constexpr Color STONE_COLOR = {128, 128, 128, 255};
constexpr Color SAND_COLOR = {194, 178, 128, 255};

static bool s_ShowClaims = true;

namespace snaps {

bool tick = false;

Block StoneBlock(int x, int y) {
    return Block {
        .WorldPosition = {static_cast<float>(x) * BOX_SIZE, static_cast<float>(y) * BOX_SIZE},
        .FillColor = STONE_COLOR,
        .IsDynamic = false
    };
}

Block SandBlock(int x, int y) {
    return Block {
        .WorldPosition = {static_cast<float>(x) * BOX_SIZE, static_cast<float>(y) * BOX_SIZE},
        .FillColor = SAND_COLOR,
        .IsDynamic = true
    };
}

void SetStone(Grid& grid, int x, int y) {
    grid.At(x, y) = StoneBlock(x, y);
}

void InitializeMap(Grid& grid) {

    const int gridWidth = GetScreenWidth() / BOX_SIZE;
    const int gridHeight = GetScreenHeight() / BOX_SIZE - 3;

    // TOP-BOTTOM border
    for (int x = 0; x <= gridWidth; x++) {
        SetStone(grid, x, 0);
        SetStone(grid, x, 1);
        SetStone(grid, x, GetScreenHeight() / BOX_SIZE - 3);
        SetStone(grid, x , GetScreenHeight() / BOX_SIZE - 4);
    }
    // LEFT-RIGHT border
    for (int y = 0; y <= gridHeight; y++) {
        SetStone(grid, 0, y);
        SetStone(grid, 1, y);
        SetStone(grid, gridWidth - 1, y);
        SetStone(grid, gridWidth - 2, y);
    }

    // shelves
    for (int x = 2; x <= 5; x++) {
        SetStone(grid, x, 6);
    }
    for (int x = 15; x <= 17; x++) {
        SetStone(grid, x, 10);
    }
    for (int x = 9; x <= 10; x++) {
        SetStone(grid, x, 10);
    }
}

void DrawUi(const Grid& grid) {
    DrawFPS(10, GetScreenHeight() - 25);

    // Claimed tiles
    if (s_ShowClaims) {
        for (int y = 0; y < grid.Height(); y++) {
            for (int x = 0; x < grid.Width(); x++) {
                const auto& block = grid.At(x, y);
                if (block.has_value()) {
                    Color color = block->IsDynamic ? BLUE : RED;
                    DrawRectangleLines(x * BOX_SIZE, y * BOX_SIZE, BOX_SIZE, BOX_SIZE, color);
                }
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
        const auto& below = grid.At(gridPosX, gridPosY+1);
        if (below.has_value() and below->WorldPosition.y < (gridPosY+1) * BOX_SIZE) return;
        grid.At(gridPosX, gridPosY) = Block {
            .WorldPosition = {static_cast<float>(worldPosX), static_cast<float>(worldPosY)},
            .FillColor = SAND_COLOR,
            .IsDynamic = true
        };
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        grid.At(gridPosX, gridPosY) = Block {
            .WorldPosition = {static_cast<float>(worldPosX), static_cast<float>(worldPosY)},
            .FillColor = STONE_COLOR,
            .IsDynamic = false
        };
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        grid.At(gridPosX, gridPosY) = std::nullopt;
    }

    if (IsKeyReleased(KEY_UP)) {
        for (auto& block : grid.Blocks()) {
            if (block and block->IsDynamic) {
                // AddForce(*block, {0, -BOX_SIZE * 2 * GRAVITY});
                const int distanceToJump = 12;
                const int pixelsToJump = BOX_SIZE * distanceToJump + 2; // +1 to add a margin
                const float jumpVelocity = std::sqrt(2 * GRAVITY * pixelsToJump);
                const Vector2 jumpImpulse = Vector2{0, -jumpVelocity} / block->InvMass;
                std::cout << "jump velocity: " << jumpVelocity << std::endl;
                std::cout << "impulse: " << jumpImpulse.y << std::endl;
                ApplyImpulse(*block, jumpImpulse);
            }
        }
    }
    if (IsKeyReleased(KEY_LEFT)) {
        for (auto& block : grid.Blocks()) {
            if (block and block->IsDynamic) {
                ApplyImpulse(*block, {-200.0f, 0});
            }
        }
    }
    if (IsKeyReleased(KEY_RIGHT)) {
        for (auto& block : grid.Blocks()) {
            if (block and block->IsDynamic) {
                ApplyImpulse(*block, {+333.f, 0});
            }
        }
    }
    if (IsKeyReleased(KEY_SPACE)) {
        tick = true;
    }

    if (IsKeyPressed(KEY_TAB)) {
        s_ShowClaims = not s_ShowClaims;
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
    constexpr int screenWidth = 320;
    constexpr int screenHeight = 320;

    InitWindow(screenWidth, screenHeight, "Snaps Sandbox");
    SetTargetFPS(60);

    snaps::Grid grid(100, 100);
    snaps::SnapsEngine engine(grid);

    InitializeMap(grid);

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginDrawing(); {
            ClearBackground(BLACK);
            HandleInput(grid);
            engine.Step(1.0f / 60.0f);
            Draw(grid);
            DrawUi(grid);
            snaps::tick = false;
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
