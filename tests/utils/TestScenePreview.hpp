#pragma once
#include "TestScene.hpp"

class TestScenePreview {
public:
    TestScenePreview(const TestScene& scene);

    void Show();

private:
    void HandleInput();

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
    void ShowZoomButtons();
    void ShowReplayButtons();
    void ShowHelpButton();
    void ShowButtons();
    void ShowHelp();

    const TestScene& m_Scene;

    // UI state
    int m_SelectedFrameIndex = 0;
    int m_ZoomLevelIndex = 0;
    bool m_IsPlaying = false;
    bool m_ShowHelp = false;
    Vector2 m_PreviewOffset = {0, 0};
};
