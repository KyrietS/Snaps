#pragma once
#include "GridCheckers.hpp"
#include "snaps/Grid.hpp"
#include "snaps/SnapsEngine.hpp"
#include <map>

#define EXPECT_SCENE(scene, checker) (scene).AddCheck((checker), __FILE__, __LINE__, false)
#define ASSERT_SCENE(scene, checker) do { (scene).AddCheck((checker), __FILE__, __LINE__, true); if (testing::Test::HasFatalFailure()) return; } while(0)

class TestScene {
public:
    static bool s_ShowPreviewOnFailure;
    static bool s_ShowPreviewAlways;

    TestScene(snaps::SnapsEngine& engine, snaps::Grid& grid);
    ~TestScene();

    /**
     * Advance the simulation by one tick (using the current delta time)
     */
    void Tick();

    /**
     * Advance the simulation by AT LEAST the given number of seconds (or fraction thereof).
     */
    void TickTime(float seconds);

    /**
     * Advance the simulation by a fixed number of ticks.
     * Note: always prefer TickTime() so that the test is not dependent on the delta time.
     */
    void TickN(int numberOfTicks);

    /**
     * Attempts to break into the debugger at the given tick number.
     * If no debugger is attached, probably terminates the program.
     *
     * @param tickNumber exact tick number (0-based) at which to break
     * Note: the break happens after the tick, so BreakAtTick(0) breaks after the first tick
     */
    void BreakAtTick(int tickNumber);

    /**
     * Sets the delta time used for each tick. Use only before any Tick*() call.
     * Default is 1/60 (60 FPS).
     */
    void SetDeltaTime(float newDeltaTime);

    /**
     * Returns the current delta time.
     */
    float GetDeltaTime() const;

    const snaps::Grid& GetCurrentGrid() const;
    const std::deque<snaps::Grid>& GetGridHistory() const;
    const std::map<std::size_t, std::vector<CheckResult>>& GetCheckResults() const;

    void AddCheck(const GridChecker& checker, const char* file, int line, bool fatal = false);
private:
    bool HasAnyFailedChecks() const;

    snaps::SnapsEngine& m_Engine;
    snaps::Grid& m_Grid;

    float m_DeltaTime = 1.0f / 60.0f;
    std::optional<int> m_BreakAtTick;
    std::deque<snaps::Grid> m_GridHistory;
    std::map<std::size_t, std::vector<CheckResult>> m_CheckResults = {{0, {}}}; // frame number -> results (can be empty)
};

