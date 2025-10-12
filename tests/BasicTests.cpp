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

    m_Scene->Tick();
    // EXPECT_SCENE(m_Scene, check::BlockIsMovingDownAt(2, 2)); FIXME it collides mid-air with block below
    EXPECT_SCENE(m_Scene, check::BlockIsMovingDownAt(2, 3));

    m_Scene->TickTime(0.5);
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 2));
}

// TODO Test: Block stopped mid-tile should be snapped (aligned)

// TODO Test impulse: Impulse too weak to slide due to friction
// TODO Test impulse: Impulse too weak to jump due to gravity
// TODO Test impulse: Impulse too strong that exceeds max speed limit
// TODO Test impulse: Impulse to the right when in mid-air
// TODO Test impulse: Impulse angular
// TODO Test impulse: Impulse stops block mid-tile

// TODO Test slide: Slide and stop due to friction
// TODO Test slide: Slide and stop due hit solid block
// TODO Test slide: Slide and stop due hit dynamic block
// TODO Test slide: Slide 3 blocks next to each other
// TODO Test slide: Slide 3 blocks on top of each other
// TODO Test slide: Slide over the edge and fall vertically due to wall
// TODO Test slide: Slide and fall into the hole

// TODO Test projection: Horizontal projection hit wall and fall
// TODO Test projection: Horizontal projection hit floor slide and stop
// TODO Test projection: angular projection hit wall go up and fall
// TODO Test projection: angular projection hit ceiling and fall

// TODO Test edge: Go over the edge going right and down
// TODO Test edge: Go over the edge going left and down
// TODO Test edge: Go over the edge going right and up
// TODO Test edge: Go over the edge going left and up
// TODO Test edge: Go under the edge going right and down
// TODO Test edge: Go under the edge going left and down
// TODO Test edge: Go under the edge going right and up
// TODO Test edge: Go under the edge going left and up

// TODO Test collision of 2: Move 2 blocks horizontally from the opposite directions and stop due to collision
// TODO Test collision of 2: Move 2 blocks horizontally in the same direction and stop due to collision (one block is slower)
// TODO Test collision of 2: Move 2 blocks vertically from the opposite directions and stop due to collision
// TODO Test collision of 2: Move 2 blocks vertically in the same direction and stop due to collision

// TODO Test friction: Greater mass means stronger friction
// TODO Test friction: Slide when bottom block has no friction
// TODO Test friction: Slide when upper block has no friction
// TODO Test friction: Slide between blocks with different friction
// TODO Test friction: No friction when 2 blocks are sliding on each other

// TODO Test grid bounds: Block stops at bottom
// TODO Test grid bounds: Block stops at top
// TODO Test grid bounds: Block stops at left
// TODO Test grid bounds: Block stops at right