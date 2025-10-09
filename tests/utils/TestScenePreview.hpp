#pragma once
#include "TestScene.hpp"

class TestScenePreview {
public:
    explicit TestScenePreview(const TestScene& scene);

    void Show();

private:
    void HandleInput();

    // Preview
    Camera2D GetPreviewCamera() const;

    void HandlePreviewInput();
    void DrawFramePreview();

    void ShowFramePreview();
    void ShowFramePaginationBar();
    void PlaySimulation();

    const snaps::Grid& GetSelectedGrid() const;
    void SelectNextFrame();
    void SelectPreviousFrame();
    void SelectFirstFrame();
    void SelectLastFrame();
    bool IsAtLastFrame() const;

    float GetZoomLevel() const;

    // UI
    void ShowGui();
    void ShowButtons();
    void ShowHelp() const;
    void ShowZoomButtons();
    void ShowReplayButtons();
    void ShowHelpButton();

    bool AreAllGridsSameSize() const;

    const TestScene& m_Scene;

    // UI state
    int m_SelectedFrameIndex = 0;
    std::optional<std::pair<int, int>> m_SelectedGridPosition = std::nullopt;
    std::optional<std::pair<int, int>> m_HoveredGridPosition = std::nullopt;
    int m_ZoomLevelIndex = 0;
    bool m_IsPlaying = false;
    bool m_ShowHelp = false;
    Vector2 m_PreviewOffset = {0, 0};
};
