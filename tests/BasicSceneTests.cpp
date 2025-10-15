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

TEST_F(BasicSceneTest, BlockSpawnedMidTileShouldBeAligned) {
    InitializeTestScene(5, 5);
    AddSand(2, 3); // spawn on ground
    GetBlock(2, 3).WorldPosition.x += snaps::BOX_SIZE / 2; // move right mid-tile

    EXPECT_SCENE(m_Scene, check::Not(check::BlockIsAlignedAt(2, 3)));
    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
}

TEST_F(BasicSceneTest, BlockStoppedMidTileShouldBeAligned) {
    InitializeTestScene(5, 5);
    AddSand(1, 1);
    GetBlock(1, 1).Velocity.x = snaps::BOX_SIZE;

    m_Scene->TickTime(0.5);

    constexpr int finalPosX = 2;
    constexpr int finalPosY = 3;
    EXPECT_SCENE(m_Scene, check::Not(check::BlockIsAlignedAt(finalPosX, finalPosY)));

    GetBlock(finalPosX, finalPosY).Velocity.x = 0; // horizontal movement stopped mid-tile
    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsXAlignedAt(finalPosX, finalPosY)); // X aligned, Y not aligned (yet)
    EXPECT_SCENE(m_Scene, check::Not(check::BlockIsAlignedAt(finalPosX, finalPosY)));

    m_Scene->TickTime(0.2); // total alignment (snapping to grid) should happen during this time
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(finalPosX, finalPosY));
}

struct ImpulseTest : SceneTest {};

TEST_F(ImpulseTest, ImpulseTooWeakToSlideDueToFriction) {
    InitializeTestScene(5, 5);
    AddSand(2, 3);

    snaps::ApplyImpulse(GetBlock(2, 3), {100.0f, 0.0f});

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsXAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 3));
}

TEST_F(ImpulseTest, ImpulseTooWeakToJumpDueToGravity) {
    InitializeTestScene(5, 5);
    AddSand(2, 3);

    snaps::ApplyImpulse(GetBlock(2, 3), {0.0f, -100.0f});

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 3));
}

TEST_F(ImpulseTest, ImpulseTooStrongThatExceedsMaxSpeedLimit) {
    InitializeTestScene(5, 5);
    AddSand(1, 2);

    const float maxSpeed = snaps::BOX_SIZE / m_Scene->GetDeltaTime();
    const float maxImpulse = maxSpeed / GetBlock(1, 2).InvMass;
    snaps::ApplyImpulse(GetBlock(1, 2), {maxImpulse + 10.0f, 0.0f});

    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(1, 2));
    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(1, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(2, 3));

    // FIXME: Generally the behavior of such fast moving blocks is currently undefined.
    //        The engine should either clamp the velocity or apply sub-stepping.
    //        This check should never pass because it means that block claims (2,3) but went past it.
    // EXPECT_SCENE(m_Scene, check::BlockWorldPositionIsAt(2, 3, check::Vector(testing::Lt(2 * snaps::BOX_SIZE), testing::_)));
}

TEST_F(ImpulseTest, ImpulseToTheRightWhenInMidAir) {
    InitializeTestScene(5, 5);
    AddSand(2, 2);

    snaps::ApplyImpulse(GetBlock(2, 2), {100.0f, 0.0f});

    m_Scene->Tick(); // the block falls down and moves right
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingDownAt(3, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingRightAt(3, 3));

    m_Scene->TickTime(0.5); // block should eventually land on the ground
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(3, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(3, 3));
}

TEST_F(ImpulseTest, ImpulseAngular) {
    InitializeTestScene(5, 5);
    AddSand(1, 3);

    auto& block = GetBlock(1, 3);
    snaps::ApplyImpulse(block, {50.0f, -snaps::GRAVITY });

    m_Scene->Tick(); // the block moves up and right
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(1, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingUpAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingRightAt(2, 2));

    m_Scene->TickTime(0.8f); // block should eventually land on the ground one tile to the right
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 3));
}

TEST_F(ImpulseTest, ImpulseThroughTheCorner) {
    InitializeTestScene(5, 5);
    AddWall(1, 3);
    AddWall(2, 2);
    AddSand(1, 2);
    snaps::ApplyImpulse(GetBlock(1, 2), {600.0f, 600.0f});

    m_Scene->TickTime(0.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(1, 2));
}

struct SlideTest : SceneTest {};

TEST_F(SlideTest, SlideAndStopDueToFriction) {
    InitializeTestScene(10, 5);
    AddSand(2, 3);
    GetBlock(2, 3).Friction = 0.5f;
    snaps::ApplyImpulse(GetBlock(2, 3), {400.0f, 0.0f});

    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    m_Scene->TickTime(0.7);
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(5, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(5, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(5, 3));
}

TEST_F(SlideTest, SlideAndStopDueToHitWithSolidWall) {
    InitializeTestScene(10, 5);
    AddSand(2, 3);
    AddWall(5, 3);
    GetBlock(2, 3).Friction = 0.5f;
    GetBlock(2, 3).Velocity.x = 500.0f;

    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    m_Scene->TickTime(0.7);
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(3, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(4, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(4, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(4, 3));
}

TEST_F(SlideTest, SlideAndStopDueToHitWithDynamicWall) {
    InitializeTestScene(10, 5);
    AddSand(2, 3);
    AddSand(5, 3);
    GetBlock(2, 3).Friction = 0.5f;
    GetBlock(2, 3).Velocity.x = 500.0f;

    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    m_Scene->TickTime(0.7);
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(3, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(4, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(4, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(4, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(5, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(6, 3));
}

TEST_F(SlideTest, SlideRightThreeBlocksNextToEachOther) {
    InitializeTestScene(10, 5);
    AddSand(2, 3);
    AddSand(3, 3);
    AddSand(4, 3);
    GetBlock(2, 3).Friction = 0.5f;
    GetBlock(3, 3).Friction = 0.5f;
    GetBlock(4, 3).Friction = 0.5f;
    snaps::ApplyImpulse(GetBlock(2, 3), {400.0f, 0.0f});
    snaps::ApplyImpulse(GetBlock(3, 3), {400.0f, 0.0f});
    snaps::ApplyImpulse(GetBlock(4, 3), {400.0f, 0.0f});

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingRightAt(3, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingRightAt(4, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingRightAt(5, 3));

    m_Scene->TickTime(0.7);
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(4, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(5, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(6, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(7, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(8, 3));
}

TEST_F(SlideTest, SlideLeftThreeBlocksNextToEachOther) {
    InitializeTestScene(10, 5);
    AddSand(7, 3);
    AddSand(6, 3);
    AddSand(5, 3);
    GetBlock(7, 3).Friction = 0.5f;
    GetBlock(6, 3).Friction = 0.5f;
    GetBlock(5, 3).Friction = 0.5f;
    snaps::ApplyImpulse(GetBlock(7, 3), {-400.0f, 0.0f});
    snaps::ApplyImpulse(GetBlock(6, 3), {-400.0f, 0.0f});
    snaps::ApplyImpulse(GetBlock(5, 3), {-400.0f, 0.0f});

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(7, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingLeftAt(6, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingLeftAt(5, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingLeftAt(4, 3));
    m_Scene->TickTime(0.7);
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(1, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(3, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(4, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(5, 3));
}

TEST_F(SlideTest, SlideRightThreeBlocksStacked) {
    InitializeTestScene(15, 6);
    AddSand(2, 4);
    AddSand(2, 3);
    AddSand(2, 2);
    snaps::ApplyImpulse(GetBlock(2, 4), {400.0f, 0.0f});
    snaps::ApplyImpulse(GetBlock(2, 3), {400.0f, 0.0f});
    snaps::ApplyImpulse(GetBlock(2, 2), {400.0f, 0.0f});

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsMovingRightAt(3, 4));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingRightAt(3, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingRightAt(3, 2));

    m_Scene->TickTime(1.0f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(3, 4));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(9, 4));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(13, 4));
}

TEST_F(SlideTest, SlideLeftThreeBlocksStacked) {
    InitializeTestScene(15, 6);
    AddSand(12, 4);
    AddSand(12, 3);
    AddSand(12, 2);
    snaps::ApplyImpulse(GetBlock(12, 4), {-400.0f, 0.0f});
    snaps::ApplyImpulse(GetBlock(12, 3), {-400.0f, 0.0f});
    snaps::ApplyImpulse(GetBlock(12, 2), {-400.0f, 0.0f});

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsMovingLeftAt(11, 4));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingLeftAt(11, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingLeftAt(11, 2));

    m_Scene->TickTime(1.0f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(1, 4));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(5, 4));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(11, 4));
}

TEST_F(SlideTest, SlideOverTheEdgeAndFallVerticallyDueToWall) {
    InitializeTestScene(6, 5);
    AddWall(1, 2);
    AddWall(2, 2);
    AddWall(4, 1);
    AddSand(1, 1);
    GetBlock(1, 1).Velocity.x = 300.0;

    m_Scene->TickTime(0.8f);;
    // EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(3, 3)); // FIXME: This should pass
}

TEST_F(SlideTest, SlideOverTheEdgeAndFallVerticallyDueToWall2) {
    InitializeTestScene(6, 5);
    AddWall(1, 2);
    AddWall(2, 2);
    AddWall(4, 1);
    AddWall(4, 2);
    AddSand(1, 1);
    GetBlock(1, 1).Velocity.x = 300.0;

    m_Scene->TickTime(0.8f);;
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(3, 3));
}

TEST_F(SlideTest, SlideAndFallIntoTheHole) {
    InitializeTestScene(6, 5);
    AddWall(1, 3);
    AddWall(2, 3);
    AddWall(4, 3);
    AddSand(1, 2);
    GetBlock(1, 2).Velocity.x = 500.0;

    m_Scene->TickTime(0.8f);;
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(3, 3));
}

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
// TODO Test collision of 2: Try to move 2 neighbor blocks towards each other

// TODO Test friction: Greater mass means stronger friction
// TODO Test friction: Slide when bottom block has no friction
// TODO Test friction: Slide when upper block has no friction
// TODO Test friction: Slide between blocks with different friction
// TODO Test friction: No friction when 2 blocks are sliding on each other

// TODO Test grid bounds: Block stops at bottom
// TODO Test grid bounds: Block stops at top
// TODO Test grid bounds: Block stops at left
// TODO Test grid bounds: Block stops at right