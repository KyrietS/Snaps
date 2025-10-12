#include "fixtures/SceneTest.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

struct BasicSceneTest : SceneTest {};

TEST_F(BasicSceneTest, FreeFall) {
    InitializeTestScene(5, 5);
    AddSand(2, 2);

    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsStaticAt(2, 4));

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingDownAt(2, 3));

    m_Scene->TickTime(0.5);
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 3));
}

TEST_F(BasicSceneTest, SpawnOnTheGround) {
    InitializeTestScene(5, 5);
    AddSand(2, 3);

    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsStaticAt(2, 4));
    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsStaticAt(2, 4));
}

TEST_F(BasicSceneTest, FreeFallNextToEachOther) {
    InitializeTestScene(5, 5);
    AddSand(1,1);
    AddSand(2, 1);
    AddSand(3, 1);

    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(1, 1));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 1));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(3, 1));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(1, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(3, 2));

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(1, 1));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 1));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(3, 1));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(1, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(3, 2));

    m_Scene->TickTime(0.7);
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(1, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(3, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(1, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(3, 3));
}

TEST_F(BasicSceneTest, SandFallsOnSand) {
    InitializeTestScene(5, 5);
    AddSand(2, 1);
    AddSand(2, 3);

    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 2));
    m_Scene->TickTime(0.5);

    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 3));
}

TEST_F(BasicSceneTest, FreeFallOnTopOfEachOther) {
    InitializeTestScene(5, 5);
    AddSand(2, 1);
    AddSand(2, 2);

    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 1));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 3));

    m_Scene->Tick(); // blocks fall together
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 1));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingDownAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingDownAt(2, 3));

    m_Scene->TickTime(0.5);
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 2));
}