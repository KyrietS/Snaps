#include "TestScene.hpp"

TestScene::TestScene(Snaps::SnapsEngine& engine, Snaps::Grid& grid)
	: m_Engine(engine)
	, m_Grid(grid)
{}

void TestScene::Tick() {
	Tick(1);
}

void TestScene::Tick(int numberOfTicks) {
	for (int tick = 0; tick < numberOfTicks; tick++) {
		m_GridHistory.push_back(m_Grid);
		m_Engine.Step(m_DeltaTime);
	}
}

void TestScene::SetDeltaTime(const float dt) {
	m_DeltaTime = dt;
}

const Snaps::Grid& TestScene::GetCurrentGrid() const {
	return m_Grid;
}

const std::deque<Snaps::Grid>& TestScene::GetGridHistory() const {
	return m_GridHistory;
}
