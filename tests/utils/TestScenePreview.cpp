#include "TestScenePreview.hpp"

#include <algorithm>
#include <raylib.h>

namespace {

std::pair<int, int> ToWindowCoordinates(const snaps::Block& block) {
    return {static_cast<int>(block.WorldPosition.x), static_cast<int>(block.WorldPosition.y)};
}

void DrawGrid(const snaps::Grid& grid) {
    const int width = grid.Width();
    const int height = grid.Height();

    // Draw border in the middle of the screen
    const int gridScreenX = (GetScreenWidth() - width * snaps::BOX_SIZE) / 2;
    const int gridScreenY = (GetScreenHeight() - height * snaps::BOX_SIZE) / 2;
    const int gridScreenWidth = width * snaps::BOX_SIZE;
    const int gridScreenHeight = height * snaps::BOX_SIZE;

    for (const auto& block : grid.Blocks()) {
        if (block.has_value()) {
            auto [x, y] = ToWindowCoordinates(*block);
            DrawRectangle(x + gridScreenX, y + gridScreenY, snaps::BOX_SIZE, snaps::BOX_SIZE, block->FillColor);
        }
    }

    // Draw grid outline (don't use DrawRectangleLines because corner is broken)
    DrawLine(gridScreenX, gridScreenY, gridScreenX + gridScreenWidth, gridScreenY, YELLOW);
    DrawLine(gridScreenX, gridScreenY + gridScreenHeight, gridScreenX + gridScreenWidth, gridScreenY + gridScreenHeight, YELLOW);
    DrawLine(gridScreenX, gridScreenY, gridScreenX, gridScreenY + gridScreenHeight, YELLOW);
    DrawLine(gridScreenX + gridScreenWidth, gridScreenY, gridScreenX + gridScreenWidth, gridScreenY + gridScreenHeight, YELLOW);
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
