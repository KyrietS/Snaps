#include "TestScenePreview.hpp"

#include <algorithm>
#include <raylib.h>

namespace {

std::pair<int, int> ToWindowCoordinates(const snaps::Block& block) {
    return {static_cast<int>(block.WorldPosition.x), static_cast<int>(block.WorldPosition.y)};
}

void DrawRect(const int gridScreenX, const int gridScreenY, const int gridScreenWidth, const int gridScreenHeight, const Color color) {
    DrawLine(gridScreenX, gridScreenY, gridScreenX + gridScreenWidth, gridScreenY, color);
    DrawLine(gridScreenX, gridScreenY + gridScreenHeight, gridScreenX + gridScreenWidth, gridScreenY + gridScreenHeight, color);
    DrawLine(gridScreenX, gridScreenY, gridScreenX, gridScreenY + gridScreenHeight, color);
    DrawLine(gridScreenX + gridScreenWidth, gridScreenY, gridScreenX + gridScreenWidth, gridScreenY + gridScreenHeight, color);
}

void DrawPixelGrid(const int gridWidth, const int gridHeight, const Color color) {
    constexpr int pixelSize = 1;
    for (int x = 0; x <= gridWidth; x += pixelSize) {
        for (int y = 0; y <= gridHeight; y += pixelSize) {
            DrawRect(x, y, pixelSize, pixelSize, color);
        }
    }
}

void DrawGrid(const snaps::Grid& grid) {
    const int width = grid.Width();
    const int height = grid.Height();

    // Draw border in the middle of the screen
    const int gridOffsetX = (GetScreenWidth() - width * snaps::BOX_SIZE) / 2;
    const int gridOffsetY = (GetScreenHeight() - height * snaps::BOX_SIZE) / 2;
    const int gridWidth = width * snaps::BOX_SIZE;
    const int gridHeight = height * snaps::BOX_SIZE;

    Camera2D camera = {
        .offset = Vector2{static_cast<float>(gridOffsetX) + static_cast<float>(gridWidth) / 2, static_cast<float>(gridOffsetY) + static_cast<float>(gridHeight) / 2},
        .target = Vector2{static_cast<float>(gridWidth) / 2, static_cast<float>(gridWidth) / 2},
        .rotation = 0.0f,
        .zoom = 2.0f
    };
    BeginMode2D(camera);


    Vector2 gridScreenPos = GetWorldToScreen2D({0.0f, 0.0f}, camera);
    BeginScissorMode(
        static_cast<int>(gridScreenPos.x) - 1,
        static_cast<int>(gridScreenPos.y) - 1,
        gridWidth * static_cast<int>(camera.zoom) + 2,
        gridHeight * static_cast<int>(camera.zoom) + 2);

    if (camera.zoom > 4.0f) {
        DrawPixelGrid(gridWidth, gridHeight, Color(40, 40, 40, 255));
    }

    for (const auto& block : grid.Blocks()) {
        if (block.has_value()) {
            auto [x, y] = ToWindowCoordinates(*block);
            DrawRectangle(x, y, snaps::BOX_SIZE, snaps::BOX_SIZE, block->FillColor);
        }
    }


    // Draw claimed grids
    for (int gridX = 0; gridX < width; gridX++) {
        for (int gridY = 0; gridY < height; gridY++) {
            const auto& block = grid[gridX, gridY];
            if (block.has_value() and block->IsDynamic) {
                const int x = gridX * snaps::BOX_SIZE;
                const int y = gridY * snaps::BOX_SIZE;
                DrawRect(x, y, snaps::BOX_SIZE, snaps::BOX_SIZE, RED);
            }
        }
    }

    // Draw grid outline (don't use DrawRectangleLines because corner is broken)
    DrawRect(0, 0, gridWidth, gridHeight, YELLOW);

    EndScissorMode();
    EndMode2D();
}
}

TestScenePreview::TestScenePreview(const TestScene &scene)
    : m_Scene(scene)
    , m_GridIndex(static_cast<int>(m_Scene.GetGridHistory().size()))
{}

void TestScenePreview::Show() {
    constexpr int screenWidth = 500;
    constexpr int screenHeight = 500;

    InitWindow(screenWidth, screenHeight, "Snaps Test Scene Preview");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(screenWidth, screenHeight);
    SetTargetFPS(60);

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginDrawing();
        {
            ClearBackground(BLACK);
            DrawGrid(GetSelectedGrid());
            DrawGridSelectionUi();
        }
        EndDrawing();
    }

    CloseWindow();
}

const snaps::Grid & TestScenePreview::GetSelectedGrid() const {
    if (m_GridIndex == m_Scene.GetGridHistory().size()) return m_Scene.GetCurrentGrid();
    return m_Scene.GetGridHistory().at(m_GridIndex);
}

void TestScenePreview::DrawGridSelectionUi() {
    const int historySize = static_cast<int>(m_Scene.GetGridHistory().size());
    if (IsKeyPressed(KEY_LEFT) or IsKeyPressedRepeat(KEY_LEFT)) m_GridIndex = std::max(0, m_GridIndex - 1);
    if (IsKeyPressed(KEY_RIGHT) or IsKeyPressedRepeat(KEY_RIGHT)) m_GridIndex = std::min(historySize, m_GridIndex + 1);
    if (GetMouseWheelMove() != 0) m_GridIndex = std::clamp(m_GridIndex - static_cast<int>(GetMouseWheelMove()), 0, historySize);

    const int gridIconSize = 4;
    const int iconsBarWidth = (historySize + 1) * gridIconSize * 2;
    int iconsBarCenterOffset = (GetScreenWidth() - iconsBarWidth) / 2;
    if (iconsBarWidth > GetScreenWidth()) {
        iconsBarCenterOffset = 0;
    }

    int gridIconX = gridIconSize + iconsBarCenterOffset;
    int gridIconY = gridIconSize;

    for (int i = 0; i <= historySize; i++) {
        if (gridIconX + gridIconSize > GetScreenWidth()) {
            gridIconX = gridIconSize + iconsBarCenterOffset;
            gridIconY += gridIconSize * 2;
        }
        const Color color = (i == m_GridIndex) ? GREEN : GRAY;
        DrawRectangle(gridIconX, gridIconY, gridIconSize, gridIconSize, color);
        gridIconX += gridIconSize * 2;
    }

    const int textX = 10;
    const int textY = GetScreenHeight() - 30;
    DrawText(TextFormat("%d / %d", m_GridIndex, historySize), textX, textY, 20, WHITE);
}
