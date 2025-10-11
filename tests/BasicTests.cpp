#include "fixtures/SceneTest.hpp"
#include <gtest/gtest.h>

TEST_F(SceneTest, FreeFall) {
    InitializeTestScene(5, 5);
    AddSand(2, 2);

    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsStaticAt(2, 4));

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));

    m_Scene->TickTime(0.5);
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
}

TEST_F(SceneTest, SpawnOnTheGround) {
    InitializeTestScene(5, 5);
    AddSand(2, 3);

    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsStaticAt(2, 4));
    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsStaticAt(2, 4));
}
