#include "SceneTest.hpp"
#include "utils/TestScenePreview.hpp"
#include "utils/TestGrid.hpp"
#include "GlobalConfiguration.hpp"

namespace {
constexpr Color SAND_COLOR = { 194, 178, 128, 255 };

snaps::Block SandBlock(const int x, const int y) {
    return snaps::Block {
        .WorldPosition = {static_cast<float>(x), static_cast<float>(y)},
        .FillColor = SAND_COLOR,
        .IsDynamic = true
    };
}
}

void SceneTest::InitializeTestScene(const int gridWidth, const int gridHeight) {
    m_Grid = std::make_unique<snaps::Grid>(MakeTestGrid(gridWidth, gridHeight));
    m_Engine = std::make_unique<snaps::SnapsEngine>(*m_Grid);
    m_Scene = std::make_unique<TestScene>(*m_Engine, *m_Grid);
}

void SceneTest::TearDown() {
    if (not m_Scene) return;
    if (GlobalConfiguration::Get().PreviewAlways or (m_Scene->HasAnyFailedChecks() and GlobalConfiguration::Get().PreviewOnFailure)) {
        try {
            TestScenePreview preview(*m_Scene);
            preview.Show();
        } catch (...) {
            FAIL() << "Exception thrown while showing TestScenePreview";
        }
    }
}

void SceneTest::AddSand(const int x, const int y) const {
    assert(m_Grid);
    (*m_Grid)[x, y] = SandBlock(x * snaps::BOX_SIZE, y * snaps::BOX_SIZE);
}
