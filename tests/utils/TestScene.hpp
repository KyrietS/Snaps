#pragma once
#include "snaps/Grid.hpp"
#include "snaps/SnapsEngine.hpp"

class TestScene {
public:
	TestScene(Snaps::SnapsEngine& engine, Snaps::Grid& grid);

	void Tick();
	void Tick(int numberOfTicks);
	void BreakAtTick(int tickNumber);

	void SetDeltaTime(float newDeltaTime);

	const Snaps::Grid& GetCurrentGrid() const;
	const std::deque<Snaps::Grid>& GetGridHistory() const;

private:
	Snaps::SnapsEngine& m_Engine;
	Snaps::Grid& m_Grid;

	float m_DeltaTime = 1.0f / 60.0f;
	std::optional<int> m_BreakAtTick;
	std::deque<Snaps::Grid> m_GridHistory;
};

