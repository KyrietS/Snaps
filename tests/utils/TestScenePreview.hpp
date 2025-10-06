#pragma once
#include "TestScene.hpp"

class TestScenePreview {
public:
    TestScenePreview(const TestScene& scene);

    void Show();

private:
    const snaps::Grid& GetSelectedGrid() const;
    void DrawGridSelectionUi();

    const TestScene& m_Scene;
    int m_GridIndex = 0;
};
