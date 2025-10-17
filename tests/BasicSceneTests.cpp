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
    AddSand(1, 1);
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

    m_Scene->TickTime(0.7f);
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
    GetBlock(2, 3).WorldPosition.x += snaps::BLOCK_SIZE / 2.0f; // move right mid-tile

    EXPECT_SCENE(m_Scene, check::Not(check::BlockIsAlignedAt(2, 3)));
    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
}

TEST_F(BasicSceneTest, BlockStoppedMidTileShouldBeAligned) {
    InitializeTestScene(5, 5);
    AddSand(1, 1);
    GetBlock(1, 1).Velocity.x = snaps::BLOCK_SIZE;

    m_Scene->TickTime(0.5);

    constexpr int finalPosX = 2;
    constexpr int finalPosY = 3;
    EXPECT_SCENE(m_Scene, check::Not(check::BlockIsAlignedAt(finalPosX, finalPosY)));

    GetBlock(finalPosX, finalPosY).Velocity.x = 0; // horizontal movement stopped mid-tile
    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsXAlignedAt(finalPosX, finalPosY)); // X aligned, Y not aligned (yet)
    EXPECT_SCENE(m_Scene, check::Not(check::BlockIsAlignedAt(finalPosX, finalPosY)));

    m_Scene->TickTime(0.2f); // total alignment (snapping to grid) should happen during this time
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

    const float maxSpeed = snaps::BLOCK_SIZE / m_Scene->GetDeltaTime();
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
    snaps::ApplyImpulse(block, {50.0f, -m_Engine->GetConfig().Gravity});

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
    m_Scene->TickTime(0.7f);
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
    m_Scene->TickTime(0.7f);
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
    m_Scene->TickTime(0.7f);
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

    m_Scene->TickTime(0.7f);
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
    m_Scene->TickTime(0.7f);
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

    m_Scene->TickTime(0.8f);
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

    m_Scene->TickTime(0.8f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(3, 3));
}

TEST_F(SlideTest, SlideAndFallIntoTheHole) {
    InitializeTestScene(6, 5);
    AddWall(1, 3);
    AddWall(2, 3);
    AddWall(4, 3);
    AddSand(1, 2);
    GetBlock(1, 2).Velocity.x = 500.0;

    m_Scene->TickTime(0.8f);
    EXPECT_SCENE(m_Scene, check::BlockIsDynamicAt(3, 3));
}

struct ProjectionTest : SceneTest {};

TEST_F(ProjectionTest, HorizontalProjectionHitWallAndFall) {
    InitializeTestScene(6, 5);
    AddSand(2, 1);
    AddSand(3, 1);
    GetBlock(2, 1).Velocity.x = -50.0;
    GetBlock(3, 1).Velocity.x = +50.0;

    m_Scene->TickTime(0.8f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(1, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(4, 3));
}

TEST_F(ProjectionTest, HorizontalProjectionRightHitFloorSlideAndStop) {
    InitializeTestScene(16, 5);
    AddSand(1, 2);
    GetBlock(1, 2).Velocity.x = +200.0f;

    m_Scene->TickTime(1.0f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(9, 3));
}

TEST_F(ProjectionTest, HorizontalProjectionLeftHitFloorSlideAndStop) {
    InitializeTestScene(16, 5);
    AddSand(14, 2);
    GetBlock(14, 2).Velocity.x = -200.0f;

    m_Scene->TickTime(1.0f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(6, 3));
}

TEST_F(ProjectionTest, AngularProjectionRightHitWallAndGoUp) {
    InitializeTestScene(5, 10);
    AddSand(2, 8);
    GetBlock(2, 8).Velocity = {200.0f, -200.0f};

    m_Scene->TickTime(0.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsMovingUpAt(3, 3));
    EXPECT_SCENE(m_Scene, check::Not(check::BlockIsMovingRightAt(3, 3)));

    m_Scene->TickTime(0.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsMovingDownAt(3, 3));
    EXPECT_SCENE(m_Scene, check::Not(check::BlockIsMovingRightAt(3, 3)));
}

TEST_F(ProjectionTest, AngularProjectionLeftHitWallAndGoUp) {
    InitializeTestScene(5, 10);
    AddSand(2, 8);
    GetBlock(2, 8).Velocity = {-200.0f, -200.0f};

    m_Scene->TickTime(0.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsMovingUpAt(1, 3));
    EXPECT_SCENE(m_Scene, check::Not(check::BlockIsMovingRightAt(1, 3)));

    m_Scene->TickTime(0.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsMovingDownAt(1, 3));
    EXPECT_SCENE(m_Scene, check::Not(check::BlockIsMovingRightAt(1, 3)));
}

TEST_F(ProjectionTest, AngularProjectionRightHitCeilingAndFall) {
    InitializeTestScene(10, 5);
    AddSand(2, 3);
    GetBlock(2, 3).Velocity = {100.0f, -400.0f};

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsMovingRightAt(3, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingUpAt(3, 2));
    m_Scene->TickTime(0.3f);
    EXPECT_SCENE(m_Scene, check::BlockIsMovingDownAt(4, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingRightAt(4, 2));
}

TEST_F(ProjectionTest, AngularProjectionLeftHitCeilingAndFall) {
    InitializeTestScene(10, 5);
    AddSand(7, 3);
    GetBlock(7, 3).Velocity = {-100.0f, -400.0f};

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsMovingLeftAt(6, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingUpAt(6, 2));
    m_Scene->TickTime(0.3f);
    EXPECT_SCENE(m_Scene, check::BlockIsMovingDownAt(5, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsMovingLeftAt(5, 2));
}

struct EdgeTest : SceneTest {
    EdgeTest() {
        InitializeTestScene(8, 8);
        AddWall(3, 3);
        AddWall(4, 3);
        AddWall(3, 4);
        AddWall(4, 4);
    }
};

TEST_F(EdgeTest, GoOverTheEdgeGoingRightAndDown) {
    AddSand(4, 1);
    GetBlock(4, 1).Velocity.x = 10.0f;

    m_Scene->TickTime(1.0f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(5, 6));
}

TEST_F(EdgeTest, GoOverTheEdgeGoingLeftAndDown) {
    AddSand(3, 1);
    GetBlock(3, 1).Velocity.x = -10.0f;

    m_Scene->TickTime(1.0f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 6));
}

TEST_F(EdgeTest, GoOverTheEdgeGoingRightAndUp) {
    AddSand(1, 3);
    GetBlock(1, 3).Velocity = {200.f, -100.0f};

    m_Scene->TickTime(1.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(6, 6));
}

TEST_F(EdgeTest, GoOverTheEdgeGoingLeftAndUp) {
    AddSand(6, 4);
    GetBlock(6, 4).Velocity = {-150.f, -200.0f};

    m_Scene->TickTime(1.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(1, 6));
}

TEST_F(EdgeTest, GoUnderTheEdgeGoingRightAndDown) {
    AddSand(1, 4);
    GetBlock(1, 4).Velocity = {200.f, 0.0f};

    m_Scene->TickTime(1.0f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(6, 6));
}

TEST_F(EdgeTest, GoUnderTheEdgeGoingLeftAndDown) {
    AddSand(6, 4);
    GetBlock(6, 4).Velocity = {-200.f, 0.0f};

    m_Scene->TickTime(1.0f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(1, 6));
}

TEST_F(EdgeTest, GoUnderTheEdgeGoingRightAndUp) {
    AddSand(4, 6);
    GetBlock(4, 6).Velocity = {50.f, -400.0f};

    m_Scene->TickTime(1.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(6, 6));
}

TEST_F(EdgeTest, GoUnderTheEdgeGoingLeftAndUp) {
    AddSand(3, 6);
    GetBlock(3, 6).Velocity = {-50.f, -400.0f};

    m_Scene->TickTime(1.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(1, 6));
}

struct CollisionTest : SceneTest {
    CollisionTest() {
        InitializeTestScene(8, 8);
    }
};

TEST_F(CollisionTest, HorizontalCollisionWithTwoBlocksGoingOppositeWays) {
    InitializeTestScene(10, 4);
    AddSand(1, 2);
    AddSand(8, 2);
    GetBlock(1, 2).Velocity.x = 300.0f;
    GetBlock(8, 2).Velocity.x = -300.0f;

    m_Scene->TickTime(0.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(4, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(5, 2));
}

TEST_F(CollisionTest, HorizontalCollisionWithTwoBlocksGoingTheSameWay) {
    InitializeTestScene(10, 4);
    AddSand(1, 2);
    AddSand(3, 2);
    GetBlock(1, 2).Velocity.x = 400.0f;
    GetBlock(3, 2).Velocity.x = 300.0f;

    m_Scene->TickTime(0.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(5, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(8, 2));
}

TEST_F(CollisionTest, VerticalCollisionWithTwoBlocksGoingOppositeWays) {
    InitializeTestScene(5, 10);
    AddSand(2, 1);
    AddSand(2, 8);
    GetBlock(2, 1).Velocity.y = 300.0f;
    GetBlock(2, 8).Velocity.y = -300.0f;

    m_Scene->TickTime(1.0f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 7));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 8));
}

TEST_F(CollisionTest, VerticalCollisionWithTwoBlocksGoingTheSameWay) {
    InitializeTestScene(5, 10);
    AddSand(2, 1);
    AddSand(2, 3);
    GetBlock(2, 1).Velocity.y = 100.0f;
    GetBlock(2, 3).Velocity.y = 50.0f;

    m_Scene->TickTime(0.75f);
    EXPECT_SCENE(m_Scene, check::BlockIsEmptyAt(2, 7));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 8));

    m_Scene->TickTime(0.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 7));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 8));
}

TEST_F(CollisionTest, TwoOppositeBlocksMissEachOther) {
    InitializeTestScene(5, 5);
    AddSand(1, 2);
    AddSand(3, 2);
    GetBlock(1, 2).Velocity.x = 200.0f;
    GetBlock(3, 2).Velocity.x = -200.0f;

    m_Scene->TickTime(0.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(1, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(3, 3));
}

TEST_F(CollisionTest, TryToMoveTwoBlocksTowardsEachOther) {
    InitializeTestScene(6, 4);
    AddSand(2, 2);
    AddSand(3, 2);
    GetBlock(2, 2).Velocity.x = 200.0f;
    GetBlock(3, 2).Velocity.x = -200.0f;

    m_Scene->Tick();
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(3, 2));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(3, 2));
}

struct FrictionTest : SceneTest {};

TEST_F(FrictionTest, GreaterMassGivesStrongerFriction) {
    InitializeTestScene(10, 4);
    AddSand(1, 2);
    GetBlock(1, 2).InvMass = 1 / 1.0f;
    GetBlock(1, 2).Velocity.x = 250.0f;

    m_Scene->TickTime(1.0f);
    ASSERT_SCENE(m_Scene, check::BlockIsAlignedAt(8, 2)); // goes to the end

    AddSand(1, 2);
    GetBlock(1, 2).InvMass = 1 / 2.0f; // 2 times greater mass
    GetBlock(1, 2).Velocity.x = 250.0f;

    m_Scene->TickTime(1.0f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(5, 2)); // stops earlier
}

TEST_F(FrictionTest, SlideWhenBottomBlockHasNoFriction) {
    InitializeTestScene(10, 4);
    for (int x = 0; x < 10; x++) {
        GetBlock(x, 3).Friction = 0.0f; // no friction on bottom blocks
    }
    AddSand(1, 2);
    GetBlock(1, 2).Friction = 0.5f;
    GetBlock(1, 2).Velocity.x = 50.0f;

    m_Scene->TickTime(1.0f);
    // FIXME: friction value is not taken into account yet
    // EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(6, 3)); // goes to the end
}

TEST_F(FrictionTest, SlideWhenUpperBlockHasNoFriction) {
    InitializeTestScene(10, 4);
    for (int x = 0; x < 10; x++) {
        GetBlock(x, 3).Friction = 0.5f;
    }
    AddSand(1, 2);
    GetBlock(1, 2).Friction = 0.0f;
    GetBlock(1, 2).Velocity.x = 50.0f;

    m_Scene->TickTime(1.0f);
    // FIXME: friction value is not taken into account yet
    // EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(6, 3)); // goes to the end
}

TEST_F(FrictionTest, NoFrictionWhenTwoBlocksAreSlidingOnEachOther) {
    InitializeTestScene(10, 5);
    for (int x = 0; x < 10; x++) {
        GetBlock(x, 4).Friction = 0.0f; // no friction on bottom blocks
    }
    AddSand(1, 2);
    AddSand(1, 3);
    GetBlock(1, 2).Friction = 0.5f;
    GetBlock(1, 3).Friction = 0.5f;
    GetBlock(1, 2).Velocity.x = 200.0f;
    GetBlock(1, 3).Velocity.x = 200.0f;

    m_Scene->TickTime(1.0f);
    // FIXME: friction value is not taken into account yet
    // EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(6, 3)); // goes to the end
}

struct BoundaryTest : SceneTest {
    BoundaryTest() {
        InitializeTestScene(5, 5);
        for (auto& block: m_Grid->Blocks()) {
            block.reset();
        }
    }
};

TEST_F(BoundaryTest, BlockStopsAtLeftBoundary) {
    AddWall(0, 4);
    AddWall(1, 4);
    AddWall(2, 4);
    AddSand(2, 3);
    GetBlock(2, 3).Velocity.x = -400.0f;

    m_Scene->TickTime(0.5f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(0, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(0, 3));
}

TEST_F(BoundaryTest, BlockStopsAtRightBoundary) {
    AddWall(4, 4);
    AddWall(3, 4);
    AddWall(2, 4);
    AddSand(2, 3);
    GetBlock(2, 3).Velocity.x = +400.0f;

    // TODO: Disable gravity and check if block stops at the top
    m_Scene->TickTime(0.5f);
    // EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(4, 3)); // FIXME
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(4, 3));
}

TEST_F(BoundaryTest, BlockStopsAtTopBoundary) {
    AddWall(2, 4);
    AddSand(2, 3);
    GetBlock(2, 3).Velocity.y = -600.0f;

    m_Scene->TickTime(0.8f);
    EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 3));
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 3));
}

TEST_F(BoundaryTest, BlockStopsAtBottomBoundary) {
    AddSand(2, 2);
    GetBlock(2, 2).Velocity.y = +100.0f;

    m_Scene->TickTime(0.5f);
    // EXPECT_SCENE(m_Scene, check::BlockIsAlignedAt(2, 4)); // FIXME
    EXPECT_SCENE(m_Scene, check::BlockIsNotMovingAt(2, 4));
}
