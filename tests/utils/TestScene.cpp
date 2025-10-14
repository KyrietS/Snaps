#include "TestScene.hpp"
#include "DebugBreak.hpp"
#include <gtest/gtest.h>
#include <algorithm>
#include <ranges>

TestScene::TestScene(snaps::SnapsEngine& engine, snaps::Grid& grid)
    : m_Engine(engine)
    , m_Grid(grid) {}

void TestScene::Tick() {
    TickN(1);
}

void TestScene::TickTime(const float seconds) {
    const int ticks = static_cast<int>(std::ceil(seconds / m_DeltaTime));
    assert(ticks < 1000000 && "Too many ticks requested");
    TickN(ticks);
}

void TestScene::TickN(const int numberOfTicks) {
    for (int tick = 0; tick < numberOfTicks; tick++) {
        m_GridHistory.push_back(m_Grid);
        m_CheckResults[m_GridHistory.size()] = {};
        m_Engine.Step(m_DeltaTime);
        if (m_BreakAtTick and tick == *m_BreakAtTick) {
            DEBUG_BREAK();
        }
    }
}

void TestScene::BreakAtTick(int tickNumber) {
    m_BreakAtTick = tickNumber;
}

void TestScene::AddCheck(const check::GridChecker& checker, const char* file, const int line, const bool fatal) {
    const auto result = checker.Check(GetCurrentGrid());
    m_CheckResults[m_GridHistory.size()].push_back(result);
    if (not result.Success) {
        const size_t frameNumber = m_GridHistory.size();
        if (fatal)
            FAIL_AT(file, line) << "---- [FRAME " << frameNumber << "] Assert failed: " << result.Summary;
        else
            ADD_FAILURE_AT(file, line) << "---- [FRAME " << frameNumber << "] Check failed: " << result.Summary;
    }
}

bool TestScene::HasAnyFailedChecks() const {
    for (const auto &results: m_CheckResults | std::views::values) {
        if (std::ranges::any_of(results, [](const auto& result) { return not result.Success; })) {
            return true;
        }
    }

    return false;
}

void TestScene::SetDeltaTime(const float dt) {
    m_DeltaTime = dt;
}

float TestScene::GetDeltaTime() const {
    return m_DeltaTime;
}

const snaps::Grid& TestScene::GetCurrentGrid() const {
    return m_Grid;
}

const std::deque<snaps::Grid>& TestScene::GetGridHistory() const {
    return m_GridHistory;
}

const std::map<std::size_t, std::vector<check::CheckResult>>& TestScene::GetCheckResults() const {
    return m_CheckResults;
}
