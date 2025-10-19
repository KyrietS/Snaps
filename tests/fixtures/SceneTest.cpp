#include "SceneTest.hpp"
#include "utils/TestScenePreview.hpp"
#include "utils/TestGrid.hpp"
#include "GlobalConfiguration.hpp"

namespace {
constexpr Color SAND_COLOR = { 194, 178, 128, 255 };
constexpr Color STONE_COLOR = {128, 128, 128, 255};

snaps::Block SandBlock(const int x, const int y) {
    return snaps::Block {
        .WorldPosition = {static_cast<float>(x), static_cast<float>(y)},
        .FillColor = SAND_COLOR,
        .IsDynamic = true
    };
}

snaps::Block StoneBlock(const int x, const int y) {
    return snaps::Block {
        .WorldPosition = {static_cast<float>(x), static_cast<float>(y)},
        .FillColor = STONE_COLOR,
        .IsDynamic = false
    };
}
std::string GetTestName() {
    const auto* testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
    if (not testInfo) return "<unknown test>";
    return std::format("{}.{}", testInfo->test_suite_name(), testInfo->name());
}
}

void SceneTest::InitializeTestScene(const int gridWidth, const int gridHeight) {
    m_Grid = std::make_unique<snaps::Grid>(MakeTestGrid(gridWidth, gridHeight));
    m_Engine = std::make_unique<snaps::SnapsEngine>(*m_Grid);
    m_Scene = std::make_unique<TestScene>(*m_Engine, *m_Grid);

    m_Engine->GetConfig() = snaps::Config {
        .Gravity = 200.0f,
        .Drag = 0.0f,
    };
}

void SceneTest::TearDown() {
    if (not m_Scene) return;
    if (GlobalConfiguration::Get().PreviewAlways or (m_Scene->HasAnyFailedChecks() and GlobalConfiguration::Get().PreviewOnFailure)) {
        try {
            TestScenePreview preview(*m_Scene, GetTestName());
            preview.Show();
        } catch (...) {
            FAIL() << "Exception thrown while showing TestScenePreview";
        }
    }
}

void SceneTest::AddSand(const int x, const int y) const {
    auto& block = GetBlockOpt(x, y);
    block = SandBlock(x * snaps::BLOCK_SIZE, y * snaps::BLOCK_SIZE);
}

void SceneTest::AddWall(int x, int y) const {
    auto& block = GetBlockOpt(x, y);
    block = StoneBlock(x * snaps::BLOCK_SIZE, y * snaps::BLOCK_SIZE);
}

snaps::Block & SceneTest::GetBlock(const int x, const int y) const {
    auto& blockOpt = GetBlockOpt(x, y);
    assert(blockOpt.has_value());
    return blockOpt.value();
}

std::optional<snaps::Block>& SceneTest::GetBlockOpt(const int x, const int y) const {
    assert(m_Grid);
    assert(x >= 0 and x < m_Grid->Width());
    assert(y >= 0 and y < m_Grid->Height());
    return m_Grid->At(x, y);
}
