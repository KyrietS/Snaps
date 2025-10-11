#pragma once
#include "GridChecker.hpp"
#include "snaps/Grid.hpp"
#include "snaps/SnapsEngine.hpp"
#include <map>

#define EXPECT_SCENE(scene, checker) (scene).AddCheck((checker), __FILE__, __LINE__, false)
#define ASSERT_SCENE(scene, checker) do { (scene).AddCheck((checker), __FILE__, __LINE__, true); if (testing::Test::HasFatalFailure()) return; } while(0)

class TestScene {
public:
    static bool s_ShowPreviewOnFailure;

    TestScene(snaps::SnapsEngine& engine, snaps::Grid& grid);
    ~TestScene();

    void Tick();
    void Tick(int numberOfTicks);
    void BreakAtTick(int tickNumber);

    void SetDeltaTime(float newDeltaTime);
    float GetDeltaTime() const;

    const snaps::Grid& GetCurrentGrid() const;
    const std::deque<snaps::Grid>& GetGridHistory() const;
    const std::map<std::size_t, std::vector<CheckResult>>& GetCheckResults() const;

    void AddCheck(const GridChecker& checker, const char* file, int line, bool fatal = false);
private:
    snaps::SnapsEngine& m_Engine;
    snaps::Grid& m_Grid;

    float m_DeltaTime = 1.0f / 60.0f;
    std::optional<int> m_BreakAtTick;
    std::deque<snaps::Grid> m_GridHistory;
    std::map<std::size_t, std::vector<CheckResult>> m_CheckResults = {{0, {}}}; // frame number -> results (can be empty)
};

