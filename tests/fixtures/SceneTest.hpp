#pragma once
#include "snaps/Grid.hpp"
#include "snaps/SnapsEngine.hpp"
#include "utils/TestScene.hpp"
#include <gtest/gtest.h>

struct SceneTest : ::testing::Test {
    void InitializeTestScene(int gridWidth, int gridHeight);
    void TearDown() override;
    void AddSand(int x, int y) const;
    snaps::Block& GetBlock(int x, int y) const;
    std::optional<snaps::Block>& GetBlockOpt(int x, int y) const;

    std::unique_ptr<snaps::Grid> m_Grid;
    std::unique_ptr<snaps::SnapsEngine> m_Engine;
    std::unique_ptr<TestScene> m_Scene;
};