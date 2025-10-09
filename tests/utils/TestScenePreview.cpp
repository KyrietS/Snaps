#include "TestScenePreview.hpp"

#include <algorithm>
#include <cassert>
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
    , m_SelectedFrameIndex(static_cast<int>(m_Scene.GetGridHistory().size())) {
    assert(AreAllGridsSameSize() && "Scenes with different grid sizes are not supported");
}

void TestScenePreview::Show() {
    constexpr int screenWidth = 500;
    constexpr int screenHeight = 500;

    InitWindow(screenWidth, screenHeight, "Snaps Test Scene Preview");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(screenWidth, screenHeight);
    SetTargetFPS(static_cast<int>(1.0f / m_Scene.GetDeltaTime()));

    // Raygui
    GuiLoadStyleDark();
    GuiEnableTooltip();

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        HandleInput();
        BeginDrawing();
        {
            ClearBackground(BLACK);
            ShowFramePreview();
            ShowFramePaginationBar();
            ShowGui();

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

    // ---------- Mouse events ----------
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) m_PreviewOffset += GetMouseDelta();
}

void TestScenePreview::ShowFramePreview() {
    HandlePreviewInput();
    const Camera2D camera = GetPreviewCamera();
    BeginMode2D(camera);
    auto [x, y] = GetWorldToScreen2D({0.0f, 0.0f}, camera) - Vector2{1, 1};
    const int scissorX = static_cast<int>(x);
    const int scissorY = static_cast<int>(y);
    const int scissorWidth = GetSelectedGrid().Width() * snaps::BOX_SIZE * static_cast<int>(camera.zoom) + 2;
    const int scissorHeight = GetSelectedGrid().Height() * snaps::BOX_SIZE * static_cast<int>(camera.zoom) + 2;
    BeginScissorMode(scissorX, scissorY, scissorWidth, scissorHeight);
    DrawFramePreview();
    EndScissorMode();
    EndMode2D();
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

void TestScenePreview::HandlePreviewInput() {
    const Camera2D camera = GetPreviewCamera();
    const snaps::Grid& grid = GetSelectedGrid();

    auto [x, y] = GetScreenToWorld2D(GetMousePosition(), camera);
    const int mouseGridX = static_cast<int>(x) / snaps::BOX_SIZE;
    const int mouseGridY = static_cast<int>(y) / snaps::BOX_SIZE;
    if (grid.InBounds(mouseGridX, mouseGridY)) {
        m_HoveredGridPosition = std::make_pair(mouseGridX, mouseGridY);
    } else {
        m_HoveredGridPosition = std::nullopt;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        m_SelectedGridPosition = m_HoveredGridPosition;
    }
}

void TestScenePreview::DrawFramePreview() {
    Camera2D camera = GetPreviewCamera();
    const snaps::Grid& grid = GetSelectedGrid();
    const int width = grid.Width();
    const int height = grid.Height();
    const int gridWidth = width * snaps::BOX_SIZE;
    const int gridHeight = height * snaps::BOX_SIZE;

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

    // Draw hovered grid
    if (m_HoveredGridPosition.has_value()) {
        const int x = m_HoveredGridPosition->first * snaps::BOX_SIZE;
        const int y = m_HoveredGridPosition->second * snaps::BOX_SIZE;
        DrawRect(x, y, snaps::BOX_SIZE, snaps::BOX_SIZE, YELLOW);
    }

    // Draw selected grid
    if (m_SelectedGridPosition.has_value()) {
        const int x = m_SelectedGridPosition->first * snaps::BOX_SIZE;
        const int y = m_SelectedGridPosition->second * snaps::BOX_SIZE;
        DrawRectangle(x, y, snaps::BOX_SIZE, snaps::BOX_SIZE, Color(255, 255, 0, 70));
    }

    // Draw grid outline (don't use DrawRectangleLines because corner is broken)
    DrawRect(0, 0, gridWidth, gridHeight, YELLOW);
}

Camera2D TestScenePreview::GetPreviewCamera() const {
    const snaps::Grid& grid = GetSelectedGrid();
    const int width = grid.Width();
    const int height = grid.Height();

    const Vector2 previewPos = {static_cast<float>(GetScreenWidth()) / 2, static_cast<float>(GetScreenHeight()) / 2};
    const int gridWidth = width * snaps::BOX_SIZE;
    const int gridHeight = height * snaps::BOX_SIZE;

    const Camera2D camera = {
        .offset = previewPos + m_PreviewOffset,
        .target = Vector2{static_cast<float>(gridWidth) / 2, static_cast<float>(gridHeight) / 2},
        .rotation = 0.0f,
        .zoom = GetZoomLevel()
    };

    return camera;
}

float TestScenePreview::GetZoomLevel() const {
    return m_ZoomLevelIndex == 0 ? 1.0f : std::powf( 2.0f, static_cast<float>(m_ZoomLevelIndex) + 1);
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
    m_SelectedFrameIndex = static_cast<int>(m_Scene.GetGridHistory().size());
}

bool TestScenePreview::IsAtLastFrame() const {
    return m_SelectedFrameIndex == m_Scene.GetGridHistory().size();
}

void TestScenePreview::PlaySimulation() {
    if (not m_IsPlaying) return;

    SelectNextFrame();
    if (IsAtLastFrame()) {
        m_IsPlaying = false;
    }
}

void TestScenePreview::ShowGui() {
    ShowButtons();
    ShowHelp();
}

void TestScenePreview::ShowButtons() {
    ShowZoomButtons();
    ShowReplayButtons();
    ShowHelpButton();
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
    const float buttonsBarY = static_cast<float>(GetScreenHeight()) - 35;
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

void TestScenePreview::ShowHelp() const {
    if (not m_ShowHelp) return;

    Rectangle helpLineRect = {
        .x = 10,
        .y = static_cast<float>(GetScreenHeight()) - 60,
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

bool TestScenePreview::AreAllGridsSameSize() const {
    const int width = m_Scene.GetCurrentGrid().Width();
    const int height = m_Scene.GetCurrentGrid().Height();
    return std::ranges::all_of(m_Scene.GetGridHistory(), [&](const auto& grid) {
        return grid.Width() == width and grid.Height() == height;
    });
}
