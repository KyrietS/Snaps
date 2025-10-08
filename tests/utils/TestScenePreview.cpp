#include "TestScenePreview.hpp"

#include <algorithm>
#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui/raygui.h>
#include <raygui/style_dark.h>

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
}

TestScenePreview::TestScenePreview(const TestScene &scene)
    : m_Scene(scene)
    , m_SelectedFrameIndex(static_cast<int>(m_Scene.GetGridHistory().size()))
{}

void TestScenePreview::Show() {
    constexpr int screenWidth = 500;
    constexpr int screenHeight = 500;

    InitWindow(screenWidth, screenHeight, "Snaps Test Scene Preview");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(screenWidth, screenHeight);
    SetTargetFPS(1.0f / m_Scene.GetDeltaTime());

    // Raygui
    GuiLoadStyleDark();
    GuiEnableTooltip();


    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        BeginDrawing();
        {
            HandleInput();

            ClearBackground(BLACK);
            ShowFramePreview();
            ShowButtons();
            ShowFramePaginationBar();
            ShowHelp();

            PlaySimulation();
        }
        EndDrawing();
    }

    CloseWindow();
}

void TestScenePreview::HandleInput() {
    if (IsKeyPressed(KEY_LEFT) or IsKeyPressedRepeat(KEY_LEFT)) SelectPreviousFrame();
    if (IsKeyPressed(KEY_RIGHT) or IsKeyPressedRepeat(KEY_RIGHT)) SelectNextFrame();
    if (GetMouseWheelMove() != 0) GetMouseWheelMove() > 0 ? SelectPreviousFrame() : SelectNextFrame();

    if (IsKeyPressed(KEY_SPACE)) {
        m_IsPlaying = not m_IsPlaying;
        if (m_IsPlaying and IsAtLastFrame()) {
            SelectFirstFrame();
        }
    }
    if (IsKeyPressed(KEY_HOME)) SelectFirstFrame();
    if (IsKeyPressed(KEY_END)) SelectLastFrame();

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) m_PreviewOffset += GetMouseDelta();
}

const snaps::Grid & TestScenePreview::GetSelectedGrid() const {
    if (m_SelectedFrameIndex == m_Scene.GetGridHistory().size()) return m_Scene.GetCurrentGrid();
    return m_Scene.GetGridHistory().at(m_SelectedFrameIndex);
}

void TestScenePreview::SelectNextFrame() {
    const int historySize = static_cast<int>(m_Scene.GetGridHistory().size());
    m_SelectedFrameIndex = std::min(historySize, m_SelectedFrameIndex + 1);
}

void TestScenePreview::SelectPreviousFrame() {
    m_SelectedFrameIndex = std::max(0, m_SelectedFrameIndex - 1);
}

void TestScenePreview::SelectFirstFrame() {
    m_SelectedFrameIndex = 0;
}

void TestScenePreview::SelectLastFrame() {
    m_SelectedFrameIndex = m_Scene.GetGridHistory().size();
}

bool TestScenePreview::IsAtLastFrame() const {
    return m_SelectedFrameIndex == m_Scene.GetGridHistory().size();
}

float TestScenePreview::GetZoomLevel() const {
    return m_ZoomLevelIndex == 0 ? 1.0f : std::powf(static_cast<float>(m_ZoomLevelIndex) + 1, 2.0f);
}

void TestScenePreview::ShowFramePreview() {
    const snaps::Grid& grid = GetSelectedGrid();
    const int width = grid.Width();
    const int height = grid.Height();

    // Draw border in the middle of the screen
    const Vector2 previewPos = {static_cast<float>(GetScreenWidth()) / 2, static_cast<float>(GetScreenHeight()) / 2};
    const int gridWidth = width * snaps::BOX_SIZE;
    const int gridHeight = height * snaps::BOX_SIZE;

    const Camera2D camera = {
        .offset = previewPos + m_PreviewOffset,
        .target = Vector2{static_cast<float>(gridWidth) / 2, static_cast<float>(gridWidth) / 2},
        .rotation = 0.0f,
        .zoom = GetZoomLevel()
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

void TestScenePreview::ShowButtons() {
    ShowZoomButtons();
    ShowReplayButtons();
    ShowHelpButton();
}

void TestScenePreview::ShowFramePaginationBar() {
    const int historySize = static_cast<int>(m_Scene.GetGridHistory().size());
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
        const Color color = (i == m_SelectedFrameIndex) ? GREEN : GRAY;
        DrawRectangle(gridIconX - 1, gridIconY - 1, gridIconSize + 2, gridIconSize + 2, BLACK);
        DrawRectangle(gridIconX, gridIconY, gridIconSize, gridIconSize, color);
        gridIconX += gridIconSize * 2;
    }

    const int textX = 10;
    const int textY = GetScreenHeight() - 30;
    DrawText(TextFormat("%d / %d", m_SelectedFrameIndex, historySize), textX, textY, 20, WHITE);
}

void TestScenePreview::ShowHelp() {
    if (not m_ShowHelp) return;

    float textY = GetScreenHeight() - 60;
    Rectangle helpLineRect = {
        .x = 10,
        .y = static_cast<float>(GetScreenWidth()) - 60,
        .width = static_cast<float>(GetScreenWidth()) - 50,
        .height = 10
    };

    GuiDrawText("Move camera: Middle Mouse Button", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawText("Prev frame: Left / Scroll Up", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawText("Next frame: Right / Scroll Down", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawText("Last frame: End", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawText("First frame: Home", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawText("Quit: Escape", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawText("Play: Space", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
}

void TestScenePreview::PlaySimulation() {
    if (not m_IsPlaying) return;

    SelectNextFrame();
    if (IsAtLastFrame()) {
        m_IsPlaying = false;
    }
}

void TestScenePreview::ShowZoomButtons() {
    constexpr int zoomButtonsGroupHeight = 4 * (30 + 2);
    const Rectangle zoomButtonRect = {
        .x = static_cast<float>(GetScreenWidth()) - 35,
        .y = static_cast<float>(GetScreenHeight()) - zoomButtonsGroupHeight - 5 - 35,
        .width = 30,
        .height = 30
    };
    GuiToggleGroup(zoomButtonRect, "x1\nx4\nx8\nx16", &m_ZoomLevelIndex);
}

void TestScenePreview::ShowReplayButtons() {
    const float buttonsBarY = GetScreenHeight() - 35;
    const float playButtonX = static_cast<float>(GetScreenWidth()) / 2 - 15;
    if (m_IsPlaying) {
        GuiToggle({playButtonX, buttonsBarY, 30, 30}, GuiIconText(ICON_PLAYER_PAUSE, nullptr), &m_IsPlaying);
    } else {
        GuiToggle({playButtonX, buttonsBarY, 30, 30}, GuiIconText(ICON_PLAYER_PLAY, nullptr), &m_IsPlaying);
        if (m_IsPlaying and IsAtLastFrame()) {
            SelectFirstFrame();
        }
    }
    if (GuiButton({playButtonX - 35, buttonsBarY, 30, 30}, GuiIconText(ICON_PLAYER_PREVIOUS, nullptr))) {
        SelectPreviousFrame();
    }
    if (GuiButton({playButtonX + 35, buttonsBarY, 30, 30}, GuiIconText(ICON_PLAYER_NEXT, nullptr))) {
        SelectNextFrame();
    }
}

void TestScenePreview::ShowHelpButton() {
    const Rectangle helpButtonRect = {
        .x = static_cast<float>(GetScreenWidth()) - 35,
        .y = static_cast<float>(GetScreenHeight()) - 35,
        .width = 30,
        .height = 30
    };
    GuiToggle(helpButtonRect, GuiIconText(ICON_HELP, nullptr), &m_ShowHelp);
}
