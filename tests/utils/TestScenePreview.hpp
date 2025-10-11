#pragma once
#include "TestScene.hpp"

class TestScenePreview {
public:
    explicit TestScenePreview(const TestScene& scene);

    void Show();

private:
    void HandleInput();

    // Preview
    void ShowFramePreview();
    void ShowFramePaginationBar();
    void HandlePreviewInput();
    void DrawFramePreview();
    Camera2D GetPreviewCamera() const;
    float GetZoomLevel() const;

    // Frame control
    const snaps::Grid& GetSelectedGrid() const;
    void SelectNextFrame();
    void SelectPreviousFrame();
    void SelectFirstFrame();
    void SelectLastFrame();
    bool IsAtLastFrame() const;
    void PlaySimulation();

    // UI
    void ShowGui();
    void ShowButtons();
    void ShowZoomButtons();
    void ShowDiagnosticsButton();
    void ShowHelpButton();
    void ShowReplayButtons();
    void ShowTileInspection();
    void ShowHelp() const;
    void ShowHoveringGridPosition() const;

    // Utils
    bool AreAllGridsSameSize() const;
    bool HasAnyOkChecks(int frameIndex) const;
    bool HasAnyFailedChecks(int frameIndex) const;


    const TestScene& m_Scene;

    // UI state
    int m_SelectedFrameIndex = 0;
    std::optional<std::pair<int, int>> m_SelectedGridPosition = std::nullopt;
    std::optional<std::pair<int, int>> m_HoveredGridPosition = std::nullopt;
    int m_ZoomLevelIndex = 0;
    bool m_IsPlaying = false;
    bool m_ShowHelp = false;
    bool m_ShowDiagnostics = true;
    Vector2 m_PreviewOffset = {0, 0};
};
