#pragma once
#include "snaps/Grid.hpp"
#include "snaps/SnapsEngine.hpp"

class TestScene {
public:
    TestScene(snaps::SnapsEngine& engine, snaps::Grid& grid);

    void Tick();
    void Tick(int numberOfTicks);
    void BreakAtTick(int tickNumber);

    void SetDeltaTime(float newDeltaTime);

    const snaps::Grid& GetCurrentGrid() const;
    const std::deque<snaps::Grid>& GetGridHistory() const;
    float GetDeltaTime() const { return m_DeltaTime; }

private:
    snaps::SnapsEngine& m_Engine;
    snaps::Grid& m_Grid;

    float m_DeltaTime = 1.0f / 60.0f;
    std::optional<int> m_BreakAtTick;
    std::deque<snaps::Grid> m_GridHistory;
};

