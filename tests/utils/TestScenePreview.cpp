#include "TestScenePreview.hpp"

#include <raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui/raygui.h>
#include <raygui/style_dark.h>

#include <cmath>
#include <algorithm>
#include <cassert>
#include <utility>

namespace {
std::pair<int, int> ToWindowCoordinates(const snaps::Block& block) {
    return {static_cast<int>(block.WorldPosition.x), static_cast<int>(block.WorldPosition.y)};
}

void DrawRect(const int gridScreenX, const int gridScreenY, const int gridScreenWidth, const int gridScreenHeight,
              const Color color) {
    DrawLine(gridScreenX, gridScreenY, gridScreenX + gridScreenWidth, gridScreenY, color);
    DrawLine(gridScreenX, gridScreenY + gridScreenHeight, gridScreenX + gridScreenWidth, gridScreenY + gridScreenHeight,
             color);
    DrawLine(gridScreenX, gridScreenY, gridScreenX, gridScreenY + gridScreenHeight, color);
    DrawLine(gridScreenX + gridScreenWidth, gridScreenY, gridScreenX + gridScreenWidth, gridScreenY + gridScreenHeight,
             color);
}

void DrawPixelGrid(const int gridWidth, const int gridHeight, const Color color) {
    constexpr int pixelSize = 1;
    for (int x = 0; x <= gridWidth; x += pixelSize) {
        for (int y = 0; y <= gridHeight; y += pixelSize) {
            DrawRect(x, y, pixelSize, pixelSize, color);
        }
    }
}

void GuiDrawTextWithBg(const char* text, Rectangle textBounds, int alignment, Color tint) {
    const int textSizeX = GuiGetTextWidth(text);
    const int textSizeY = GuiGetFont().baseSize;
    constexpr Color textBackground = {0, 0, 0, 100};

    float textBgX = textBounds.x;
    if (alignment == TEXT_ALIGN_CENTER)
        textBgX = textBounds.x + (textBounds.width - static_cast<float>(textSizeX)) / 2;
    else if (alignment == TEXT_ALIGN_RIGHT)
        textBgX = textBounds.x + textBounds.width - static_cast<float>(textSizeX);

    const Rectangle textBackgroundRec = {
        textBgX - 2, textBounds.y - 2, static_cast<float>(textSizeX) + 4, static_cast<float>(textSizeY)
    };
    DrawRectangleRec(textBackgroundRec, textBackground);
    GuiDrawText(text, textBounds, alignment, tint);
}
}

TestScenePreview::TestScenePreview(const TestScene& scene, std::string title)
    : m_Scene(scene)
      , m_Title(std::move(title))
      , m_SelectedFrameIndex(static_cast<int>(m_Scene.GetGridHistory().size())) {
    assert(AreAllGridsSameSize() && "Scenes with different grid sizes are not supported");
}

void TestScenePreview::Show() {
    constexpr int screenWidth = 500;
    constexpr int screenHeight = 500;

    SetTraceLogLevel(LOG_ERROR);
    InitWindow(screenWidth, screenHeight, m_Title.c_str());
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(screenWidth, screenHeight);
    SetTargetFPS(static_cast<int>(1.0f / m_Scene.GetDeltaTime()));

    // Raygui
    GuiLoadStyleDark();
    GuiEnableTooltip();

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        HandleInput();
        BeginDrawing(); {
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
    const int scissorWidth = GetSelectedGrid().Width() * snaps::BLOCK_SIZE * static_cast<int>(camera.zoom) + 2;
    const int scissorHeight = GetSelectedGrid().Height() * snaps::BLOCK_SIZE * static_cast<int>(camera.zoom) + 2;
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
        auto color = GRAY;
        if (i == m_SelectedFrameIndex) {
            color = YELLOW;
        } else if (HasAnyFailedChecks(i)) {
            color = RED;
        } else if (HasAnyOkChecks(i)) {
            color = GREEN;
        }

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
    const int mouseGridX = x >= 0 ? static_cast<int>(x) / snaps::BLOCK_SIZE : -1;
    const int mouseGridY = y >= 0 ? static_cast<int>(y) / snaps::BLOCK_SIZE : -1;
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
    const int gridWidth = width * snaps::BLOCK_SIZE;
    const int gridHeight = height * snaps::BLOCK_SIZE;

    if (camera.zoom > 4.0f) {
        DrawPixelGrid(gridWidth, gridHeight, Color{40, 40, 40, 255});
    }

    for (const auto& block: grid.Data()) {
        if (block.has_value()) {
            auto [x, y] = ToWindowCoordinates(*block);
            DrawRectangle(x, y, snaps::BLOCK_SIZE, snaps::BLOCK_SIZE, block->FillColor);
        }
    }

    if (m_ShowDiagnostics) {
        // Draw failed checks
        std::set<std::pair<int, int> > markedPositions;
        for (const auto& result: m_Scene.GetCheckResults().at(m_SelectedFrameIndex)) {
            if (not result.Success) {
                for (const auto& [gridX, gridY]: result.Positions) {
                    if (grid.InBounds(gridX, gridY) and not markedPositions.contains({gridX, gridY})) {
                        markedPositions.emplace(gridX, gridY);
                        const int x = gridX * snaps::BLOCK_SIZE;
                        const int y = gridY * snaps::BLOCK_SIZE;
                        DrawRectangle(x, y, snaps::BLOCK_SIZE, snaps::BLOCK_SIZE, Color{255, 0, 0, 50});
                    }
                }
            }
        }

        // Draw ok checks
        for (const auto& result: m_Scene.GetCheckResults().at(m_SelectedFrameIndex)) {
            if (result.Success) {
                for (const auto& [gridX, gridY]: result.Positions) {
                    if (grid.InBounds(gridX, gridY) and not markedPositions.contains({gridX, gridY})) {
                        markedPositions.emplace(gridX, gridY);
                        const int x = gridX * snaps::BLOCK_SIZE;
                        const int y = gridY * snaps::BLOCK_SIZE;
                        DrawRectangle(x, y, snaps::BLOCK_SIZE, snaps::BLOCK_SIZE, Color{0, 255, 0, 50});
                    }
                }
            }
        }

        // Draw dynamic claims
        for (int gridX = 0; gridX < width; gridX++) {
            for (int gridY = 0; gridY < height; gridY++) {
                const auto& block = grid.At(gridX, gridY);
                if (block.has_value() and block->IsDynamic) {
                    const int x = gridX * snaps::BLOCK_SIZE;
                    const int y = gridY * snaps::BLOCK_SIZE;
                    DrawRect(x, y, snaps::BLOCK_SIZE, snaps::BLOCK_SIZE, RED);
                }
            }
        }
    }

    // Draw hovered grid
    if (m_HoveredGridPosition.has_value()) {
        const int x = m_HoveredGridPosition->first * snaps::BLOCK_SIZE;
        const int y = m_HoveredGridPosition->second * snaps::BLOCK_SIZE;
        DrawRect(x, y, snaps::BLOCK_SIZE, snaps::BLOCK_SIZE, YELLOW);
    }

    // Draw selected grid
    if (m_SelectedGridPosition.has_value()) {
        const int x = m_SelectedGridPosition->first * snaps::BLOCK_SIZE;
        const int y = m_SelectedGridPosition->second * snaps::BLOCK_SIZE;
        DrawRectangle(x, y, snaps::BLOCK_SIZE, snaps::BLOCK_SIZE, Color{255, 255, 0, 70});
    }

    // Draw grid outline (don't use DrawRectangleLines because corner is broken)
    DrawRect(0, 0, gridWidth, gridHeight, YELLOW);
}

Camera2D TestScenePreview::GetPreviewCamera() const {
    const snaps::Grid& grid = GetSelectedGrid();
    const int width = grid.Width();
    const int height = grid.Height();

    const Vector2 previewPos = {static_cast<float>(GetScreenWidth()) / 2, static_cast<float>(GetScreenHeight()) / 2};
    const int gridWidth = width * snaps::BLOCK_SIZE;
    const int gridHeight = height * snaps::BLOCK_SIZE;

    const Camera2D camera = {
        .offset = previewPos + m_PreviewOffset,
        .target = Vector2{static_cast<float>(gridWidth) / 2, static_cast<float>(gridHeight) / 2},
        .rotation = 0.0f,
        .zoom = GetZoomLevel()
    };

    return camera;
}

float TestScenePreview::GetZoomLevel() const {
    return m_ZoomLevelIndex == 0 ? 1.0f : std::pow(2.0f, static_cast<float>(m_ZoomLevelIndex) + 1);
}

const snaps::Grid& TestScenePreview::GetSelectedGrid() const {
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
    ShowTileInspection();
    ShowHelp();
    ShowHoveringGridPosition();
}

void TestScenePreview::ShowButtons() {
    ShowDiagnosticsButton();
    ShowZoomButtons();
    ShowHelpButton();
    ShowReplayButtons();
}

void TestScenePreview::ShowZoomButtons() {
    constexpr int zoomButtonsGroupHeight = 4 * (30 + 2);
    const Rectangle zoomButtonRect = {
        .x = static_cast<float>(GetScreenWidth()) - 35,
        .y = static_cast<float>(GetScreenHeight()) - zoomButtonsGroupHeight - 5 - 35 - 32,
        .width = 30,
        .height = 30
    };
    GuiToggleGroup(zoomButtonRect, "x1\nx4\nx8\nx16", &m_ZoomLevelIndex);
}

void TestScenePreview::ShowDiagnosticsButton() {
    const Rectangle helpButtonRect = {
        .x = static_cast<float>(GetScreenWidth()) - 35,
        .y = static_cast<float>(GetScreenHeight()) - 35 - 34,
        .width = 30,
        .height = 30
    };
    GuiToggle(helpButtonRect, GuiIconText(ICON_LAYERS_VISIBLE, nullptr), &m_ShowDiagnostics);
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

    const float timeSinceStartInMs = m_Scene.GetDeltaTime() * static_cast<float>(m_SelectedFrameIndex);
    const Rectangle textRect = {
        .x = (static_cast<float>(GetScreenWidth()) - 100) / 2,
        .y = buttonsBarY - 18,
        .width = 100,
        .height = 10
    };
    GuiDrawTextWithBg(TextFormat("%.3f s", timeSinceStartInMs), textRect, TEXT_ALIGN_CENTER, WHITE);
}

static std::vector<std::pair<std::string, std::string> > GetInspectData(const std::optional<snaps::Block>& block) {
    if (not block.has_value()) return {std::make_pair("Empty", "")};

    auto formatFloat = [](const float value) {
        return std::format("{:.1f}", value);
    };
    auto formatBool = [](const bool value) {
        return value ? "true" : "false";
    };
    auto formatVector = [](const Vector2& vec) {
        return std::format("({:.1f}, {:.1f})", vec.x, vec.y);
    };

    std::vector<std::pair<std::string, std::string> > result;
    result.emplace_back("WorldPosition", formatVector(block->WorldPosition));
    result.emplace_back("Velocity", formatVector(block->Velocity));
    result.emplace_back("IsDynamic", formatBool(block->IsDynamic));
    result.emplace_back("InvMass", formatFloat(block->InvMass));
    result.emplace_back("Friction", formatFloat(block->Friction));
    result.emplace_back("Acceleration", formatVector(block->Acceleration));
    return result;
}

void TestScenePreview::ShowTileInspection() {
    if (not m_SelectedGridPosition) return;

    const std::optional<snaps::Block>& block = GetSelectedGrid().At(m_SelectedGridPosition->first, m_SelectedGridPosition->second);

    const std::vector<std::pair<std::string, std::string> > inspectData = GetInspectData(block);

    Rectangle inspectPanelRect = {
        .x = 10,
        .y = 10,
        .width = 220,
        .height = static_cast<float>(inspectData.size() * 15 + 35)
    };

    if (GuiWindowBox(inspectPanelRect, "Title inspection")) {
        m_SelectedGridPosition = std::nullopt;
    }
    Rectangle labelRect = {
        inspectPanelRect.x + 5, inspectPanelRect.y + 30, inspectPanelRect.width * 0.5f - 5, 10
    };
    Rectangle valueRect = {
        inspectPanelRect.x + 5 + labelRect.width, inspectPanelRect.y + 30, inspectPanelRect.width * 0.5f - 5, 10
    };

    auto drawInspectText = [&](const char* label, const char* value) {
        GuiDrawText(label, labelRect, TEXT_ALIGN_LEFT, LIGHTGRAY);
        GuiDrawText(value, valueRect, TEXT_ALIGN_RIGHT, WHITE);
        labelRect.y += 15;
        valueRect.y += 15;
    };

    for (const auto& [label, value]: inspectData) {
        drawInspectText(label.c_str(), value.c_str());
    }

    Color selectedTileColor{0, 0, 255, 255};
    if (block.has_value()) {
        const Camera2D camera = GetPreviewCamera();
        BeginMode2D(camera);
        DrawRect(static_cast<int>(block->WorldPosition.x), static_cast<int>(block->WorldPosition.y), snaps::BLOCK_SIZE, snaps::BLOCK_SIZE,
                 selectedTileColor);
        EndMode2D();
    }

    auto checkResults = m_Scene.GetCheckResults().at(m_SelectedFrameIndex);

    for (const auto& result: checkResults) {
        if (m_SelectedGridPosition and result.Positions.contains({m_SelectedGridPosition->first, m_SelectedGridPosition->second})) {
            const Rectangle resultRec = {
                .x = 10,
                .y = labelRect.y + 15,
                .width = static_cast<float>(GetScreenWidth()) - 20,
                .height = 10
            };
            const Color resultColor = result.Success ? Color{54, 247, 90, 255} : Color{255, 122, 122, 255};
            const GuiIconName resultIcon = result.Success ? ICON_OK_TICK : ICON_CROSS;
            const char* text = GuiIconText(resultIcon, result.Summary.c_str());
            GuiDrawTextWithBg(text, resultRec, TEXT_ALIGN_LEFT, resultColor);
            labelRect.y += 16;
        }
    }
}

void TestScenePreview::ShowHelp() const {
    if (not m_ShowHelp) return;

    Rectangle helpLineRect = {
        .x = 10,
        .y = static_cast<float>(GetScreenHeight()) - 60,
        .width = static_cast<float>(GetScreenWidth()) - 50,
        .height = 10
    };
    GuiDrawTextWithBg("Move camera: Middle Mouse Button", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawTextWithBg("Select tile: Right Mouse Button", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawTextWithBg("Prev frame: Left / Scroll Up", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawTextWithBg("Next frame: Right / Scroll Down", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawTextWithBg("Last frame: End", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawTextWithBg("First frame: Home", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawTextWithBg("Quit: Escape", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
    helpLineRect.y -= 20;
    GuiDrawTextWithBg("Play: Space", helpLineRect, TEXT_ALIGN_LEFT, WHITE);
}

void TestScenePreview::ShowHoveringGridPosition() const {
    if (not m_HoveredGridPosition) return;

    const Rectangle hoverPosRect = {
        .x = static_cast<float>(GetScreenWidth()) - 100,
        .y = static_cast<float>(GetScreenHeight()) - 20,
        .width = 50,
        .height = 10
    };
    const int gridX = m_HoveredGridPosition->first;
    const int gridY = m_HoveredGridPosition->second;
    GuiDrawTextWithBg(TextFormat("(%d, %d)", gridX, gridY), hoverPosRect, TEXT_ALIGN_RIGHT, WHITE);
}

bool TestScenePreview::AreAllGridsSameSize() const {
    const int width = m_Scene.GetCurrentGrid().Width();
    const int height = m_Scene.GetCurrentGrid().Height();
    return std::ranges::all_of(m_Scene.GetGridHistory(), [&](const auto& grid) {
        return grid.Width() == width and grid.Height() == height;
    });
}

bool TestScenePreview::HasAnyOkChecks(const int frameIndex) const {
    return std::ranges::any_of(m_Scene.GetCheckResults().at(frameIndex), [](const auto& result) {
        return result.Success;
    });
}

bool TestScenePreview::HasAnyFailedChecks(const int frameIndex) const {
    return std::ranges::any_of(m_Scene.GetCheckResults().at(frameIndex), [](const auto& result) {
        return not result.Success;
    });
}
