#include "snaps/SnapsEngine.hpp"
#include "snaps/Block.hpp"
#include <raymath.h>
#include <iostream>
#include <cmath>


namespace snaps {

namespace {
float MinVelocityForDistance(float deceleration, float distance = BLOCK_SIZE) {
    return std::sqrt(2.0f * deceleration * distance);
}

void StopBlockAndAlignToX(Block& block, int gridX) {
    block.WorldPosition.x = static_cast<float>(gridX) * BLOCK_SIZE;
    block.Velocity.x = 0.0f;
}

void StopBlockAndAlignToY(Block& block, int gridY) {
    block.WorldPosition.y = static_cast<float>(gridY) * BLOCK_SIZE;
    block.Velocity.y = 0.0f;
}
} // namespace

SnapsEngine::SnapsEngine(Grid& grid) : m_Grid(grid) {}

void SnapsEngine::Step(float deltaTime) {
    m_DeltaTime = deltaTime;
    SimulatePhysics();
}

void SnapsEngine::SimulatePhysics() {
    const std::size_t numOfTiles = m_Grid.Data().size();
    for (std::size_t i = 0; i < numOfTiles; i++) {
        auto& block = m_Grid.At(i);
        const auto [x, y] = m_Grid.GetXY(i);
        if (block.has_value())
            SimulateMovement(x, y, *block);
    }

    for (int y = m_Grid.Height() - 1; y >= 0; y--) {
        for (int x = 0; x < m_Grid.Width(); x++) {
            SolveGridPhysics(x, y, CollisionPass::First);
        }
        SecondPassGridPhysicsHorizontal();
    }
    SecondPassGridPhysicsVertical();
}

void SnapsEngine::SimulateMovement(const int x, const int y, Block& block) {
    if (block.IsDynamic) {
        ApplyGravity(block);
        ApplyFriction(x, y, block);
        ApplyDrag(block);
        DiscardResistanceForcesIfNecessary(x, y, block);
        Integrate(block);
    }
}

void SnapsEngine::Integrate(Block& block) {
    if (block.InvMass <= 0.0f) return;

    block.Acceleration = {
        block.ForceAccum.x * block.InvMass,
        block.ForceAccum.y * block.InvMass
    };

    block.Velocity += block.Acceleration * m_DeltaTime;

    // Realistically, any velocity smaller than 1.0/DeltaTime will not move the object in a pixel space.
    if (std::abs(block.Velocity.x) < 0.01f) block.Velocity.x = 0.0f;
    if (std::abs(block.Velocity.y) < 0.01f) block.Velocity.y = 0.0f;

    block.WorldPosition += block.Velocity * m_DeltaTime;
    block.ForceAccum = {0, 0};

    block.NeedsCollisionResolution = true;
}

void SnapsEngine::SolveGridPhysics(int x, int y, const CollisionPass collisionPass) {
    auto& block = m_Grid.At(x, y);
    if (not block.has_value() or not block->IsDynamic or not block->NeedsCollisionResolution) return;
    SolveGridPhysics(x, y, *block, collisionPass);
}

void SnapsEngine::SolveGridPhysics(const int gridX, const int gridY, Block& block, const CollisionPass collisionPass) {
    MovementResolution resolution {gridX, gridY, collisionPass};

    if (collisionPass != CollisionPass::SecondVertical) {
        SolveMovementHorizontal(block, resolution);
        if (resolution.Resolved) return;
    }

    assert(m_Grid.At(resolution.X, resolution.Y).has_value());
    auto& movedBlock = *m_Grid.At(resolution.X, resolution.Y);

    SolveMovementVertical(movedBlock, resolution);
    if (resolution.Resolved) return;

    // Mark as resolved so we don't try to resolve it again this frame.
    movedBlock.NeedsCollisionResolution = false;
}


void SnapsEngine::SecondPassGridPhysicsHorizontal() {
    while (not m_RightMovementContacts.empty()) {
        auto [x, y] = m_RightMovementContacts.top();
        SolveGridPhysics(x, y, CollisionPass::SecondHorizontal);
        m_RightMovementContacts.pop();
    }
}

void SnapsEngine::SecondPassGridPhysicsVertical() {
    while (not m_UpMovementContacts.empty()) {
        auto [x, y] = m_UpMovementContacts.top();
        SolveGridPhysics(x, y, CollisionPass::SecondVertical);
        m_UpMovementContacts.pop();
    }
}

void SnapsEngine::SolveMovementHorizontal(Block& block, MovementResolution& resolution) {
    if (block.Velocity.x >= 0)
        SolveMovementRight(block, resolution);
    else
        SolveMovementLeft(block, resolution);
}

void SnapsEngine::SolveMovementVertical(Block& block, MovementResolution& resolution) {
    if (block.Velocity.y <= 0)
        SolveMovementUp(block, resolution);
    else
        SolveMovementDown(block, resolution);
}

void SnapsEngine::SolveMovementLeft(Block& block, MovementResolution& resolution) {
    const int x = resolution.X;
    const int y = resolution.Y;

    // Block is not moving left
    if (block.Velocity.x >= 0) return; // moving right

    const int desiredXGrid = std::floor(block.WorldPosition.x / BLOCK_SIZE);
    const bool wantsToMoveLeft = desiredXGrid < x and block.Velocity.x < 0;

    // No blocks to the left
    if (not m_Grid.InBounds(x-1, y)) {
        if (wantsToMoveLeft) {
            StopBlockAndAlignToX(block, x);
        }
        return;
    }

    auto& blockLeft = m_Grid.At(x - 1, y);

    // Desired grid is occupied. Stop.
    if (wantsToMoveLeft and blockLeft.has_value()) {
        StopBlockAndAlignToX(block, x);
        return;
    }

    // Desired grid is free. Claim it if we have enough velocity to reach it.
    if (wantsToMoveLeft and not blockLeft.has_value()) {
        // Claim a block to the right only if the center of the block can reach it (smooth edge overlapping).
        // We basically slide the block vertically until its center point does not exceed the edge.
        const float blockCenterY = block.WorldPosition.y + BLOCK_SIZE / 2;
        const int blockCenterYGrid = std::floor(blockCenterY / BLOCK_SIZE);
        const auto& blockLeftCenter = blockCenterYGrid == y ? blockLeft : m_Grid.At(x - 1, blockCenterYGrid);
        if (blockLeftCenter.has_value()) {
            StopBlockAndAlignToX(block, x);
            return;
        }

        const float deceleration = block.Acceleration.x > 0 ? block.Acceleration.x : 0.0f;
        const float minVelocityToReachNextGrid = MinVelocityForDistance(deceleration);

        // Not enough velocity to reach the next grid. Stop and align to grid.
        if (std::abs(block.Velocity.x) < minVelocityToReachNextGrid) {
            StopBlockAndAlignToX(block, x);
        } else { // Claim grid to the left.
            blockLeft = block;
            m_Grid.Remove(x, y);
            resolution.X -= 1;
        }
    }
}


void SnapsEngine::SolveMovementRight(Block& block, MovementResolution& resolution) {
    const int x = resolution.X;
    const int y = resolution.Y;

    // Block is not moving right
    if (block.Velocity.x < 0) return; // moving left

    const int desiredXGrid = static_cast<int>(block.WorldPosition.x + BLOCK_SIZE) / BLOCK_SIZE;
    const bool wantsToMoveRight = desiredXGrid > x and block.Velocity.x > 0;

    // No blocks to the right
    if (not m_Grid.InBounds(x+1, y)) {
        if (wantsToMoveRight) {
            StopBlockAndAlignToX(block, x);
        }
        return;
    }

    auto& blockRight = m_Grid.At(x + 1, y);

    // Desired grid is occupied. Stop.
    if (wantsToMoveRight and blockRight.has_value()) {
        const bool blockRightIsMoving = blockRight->Velocity.x != 0 or blockRight->Velocity.y != 0;
        if (blockRightIsMoving and resolution.Pass != CollisionPass::SecondHorizontal) { // Try in the second pass. If we are lucky, the block on
            m_RightMovementContacts.push({x, y});                      // the right will claim another block and release this one.
            resolution.Resolved = true;
        } else { // Stop.
            StopBlockAndAlignToX(block, x);
        }
        return;
    }

    // Desired grid is free. Claim it if we have enough velocity to reach it.
    if (wantsToMoveRight and not blockRight.has_value()) {
        // Claim a block to the right only if the center of the block can reach it (smooth edge overlapping).
        // We basically slide the block vertically until its center point does not exceed the edge.
        const float blockCenterY = block.WorldPosition.y + BLOCK_SIZE / 2;
        const int blockCenterYGrid = std::floor(blockCenterY / BLOCK_SIZE);
        const auto& blockRightCenter = blockCenterYGrid == y ? blockRight : m_Grid.At(x + 1, blockCenterYGrid);
        if (blockRightCenter.has_value() and blockCenterY > blockRightCenter->WorldPosition.y and blockCenterY < blockRightCenter->WorldPosition.y + BLOCK_SIZE) {
            if (resolution.Pass != CollisionPass::SecondHorizontal) {
                m_RightMovementContacts.push({x, y});
                resolution.Resolved = true;
            } else {
                StopBlockAndAlignToX(block, x);
            }
            return;
        }

        const float deceleration = block.Acceleration.x < 0 ? -block.Acceleration.x : 0.0f;
        const float minVelocityToReachNextGrid = MinVelocityForDistance(deceleration);

        // Not enough velocity to reach next grid. Stop and align to grid.
        if (std::abs(block.Velocity.x) < minVelocityToReachNextGrid) {
            StopBlockAndAlignToX(block, x);
            return;
        } else if (block.Velocity.x > 0) { // Claim grid to the right.
            blockRight = block;
            m_Grid.Remove(x, y);
            resolution.X += 1;
        }
    }

    // ALIGN POSITION TO GRID IF NECESSARY
    const float distanceFromCorrectPosition = static_cast<float>(x * BLOCK_SIZE) - block.WorldPosition.x;
    // Block stopped before reaching end of its own grid.
    if (block.Velocity.x == 0 and block.Acceleration.x == 0 and distanceFromCorrectPosition != 0.0f) {
        std::cout << "block stopped without reaching end of its own grid\n";
        // Snap it to the grid in one shot.
        StopBlockAndAlignToX(block, x);
    }
}

void SnapsEngine::SolveMovementDown(Block& block, MovementResolution& resolution) {
    const int x = resolution.X;
    const int y = resolution.Y;

    // Block is not moving down
    if (block.Velocity.y <= 0) return;

    const int desiredYGrid = static_cast<int>(block.WorldPosition.y + BLOCK_SIZE) / BLOCK_SIZE;
    const bool wantsToMoveDown = desiredYGrid > y;

    // No block below
    if (not m_Grid.InBounds(x, y+1)) {
        if (wantsToMoveDown) {
            StopBlockAndAlignToY(block, y);
        }
        return;
    }

    auto& blockBelow = m_Grid.At(x, y+1);

    // Desired grid is occupied. Stop.
    if (wantsToMoveDown and blockBelow.has_value()) {
        StopBlockAndAlignToY(block, y);
        return;
    }

    // Desired grid is free, claim it. Assume we will always have enough velocity to reach it due to gravity.
    if (wantsToMoveDown and not blockBelow.has_value()) {
        // Claim a block below only if the center of the block can reach it (smooth edge overlapping).
        // We basically slide the block horizontally until its center point does not exceed the edge.
        const float blockCenterX = block.WorldPosition.x + BLOCK_SIZE / 2;
        const int blockCenterXGrid = std::floor(blockCenterX / BLOCK_SIZE);
        const auto& blockBelowCenter = blockCenterXGrid == x ? blockBelow : m_Grid.At(blockCenterXGrid, y+1);
        if (blockBelowCenter.has_value()) {
            StopBlockAndAlignToY(block, y);
            return;
        }

        blockBelow = block;
        m_Grid.Remove(x, y);
        resolution.Y += 1;
        return;
    }

    // Only accelerated movements mid-air collision can result with a stop because
    // the gravity will make the block fall again to the desired spot.
    if (blockBelow and block.WorldPosition.y + BLOCK_SIZE >= blockBelow->WorldPosition.y) {
        // Collided with a block below. If it goes the same direction use its speed do continue down. Otherwise, stop.
        block.Velocity.y = blockBelow->Velocity.y >= 0 ? blockBelow->Velocity.y : 0;
    }
}

void SnapsEngine::SolveMovementUp(Block& block, MovementResolution& resolution) {
    const int x = resolution.X;
    const int y = resolution.Y;
    // Block is not moving up
    if (block.Velocity.y > 0) return; // moving down

    const int desiredYGrid = std::floor(block.WorldPosition.y / BLOCK_SIZE);
    const bool wantsToMoveUp = desiredYGrid < y;

    // No blocks above
    if (not m_Grid.InBounds(x, y-1)) {
        if (wantsToMoveUp) {
            StopBlockAndAlignToY(block, y);
        }
        return;
    }

    auto& blockAbove = m_Grid.At(x, y - 1);

    // Desired grid is occupied.
    if (wantsToMoveUp and blockAbove.has_value()) {
        const bool blockAboveIsMoving = blockAbove->Velocity.x != 0 or blockAbove->Velocity.y != 0;
        if (blockAboveIsMoving and resolution.Pass != CollisionPass::SecondVertical) { // Try in the second pass. If we are lucky, the block above
            m_UpMovementContacts.push({x, y});                         // will claim another block and release this one.
            resolution.Resolved = true;
        } else { // Stop.
            StopBlockAndAlignToY(block, y);
        }
        return;
    }

    // Desired grid is free. Claim it if we have enough velocity to reach it.
    if (wantsToMoveUp and not blockAbove.has_value()) {
        // Claim a block above only if the center of the block can reach it (smooth edge overlapping).
        // We basically slide the block horizontally until its center point does not exceed the edge.
        const float blockCenterX = block.WorldPosition.x + BLOCK_SIZE / 2;
        const int blockCenterXGrid = std::floor(blockCenterX / BLOCK_SIZE);
        const auto& blockAboveCenter = blockCenterXGrid == x ? blockAbove : m_Grid.At(blockCenterXGrid, y-1);
        if (blockAboveCenter.has_value() and blockCenterX > blockAboveCenter->WorldPosition.x and blockCenterX < blockAboveCenter->WorldPosition.x + BLOCK_SIZE) {
            if (resolution.Pass != CollisionPass::SecondVertical) {
                m_UpMovementContacts.push({x, y});
                resolution.Resolved = true;
            } else {
                StopBlockAndAlignToY(block, y);
            }
            return;
        }

        // More accurate check if next grid is reachable
        // const float distanceToReachNextGrid = block.WorldPosition.y - desiredYGrid * BOX_SIZE;
        // const float minVelocityToReachNextGrid = std::sqrt(2.0f * GRAVITY * distanceToReachNextGrid);
        const float deceleration = block.Acceleration.y > 0 ? block.Acceleration.y : 0.0f;
        const float minVelocityToReachNextGrid = MinVelocityForDistance(deceleration);

        // Not enough velocity to reach next grid. Stop.
        if (std::abs(block.Velocity.y) < minVelocityToReachNextGrid) {
            StopBlockAndAlignToY(block, y);
        } else { // Claim grid above.
            blockAbove = block;
            m_Grid.Remove(x, y);
            resolution.Y -= 1;
        }
    }
}

void SnapsEngine::ApplyGravity(Block& block) {
    block.ForceAccum += Vector2{0.0f, m_Config.Gravity} * block.GravityScale / block.InvMass;
}

void SnapsEngine::ApplyFriction(const int x, const int y, Block& block) {
    if (not m_Grid.InBounds(x, y+1)) return;
    if (not m_Grid.InBounds(x, y-1)) return;

    const auto& surface = block.ForceAccum.y > 0 ? m_Grid.At(x, y+1) : m_Grid.At(x, y-1);
    const bool isSliding = surface.has_value()                                // Block below must exist
        and AreTouching(block, *surface)                                      // Block must touch the surface
        and surface->Velocity.x == 0                                          // Block below must be stationary in X
        and block.Velocity.y >= 0;                                            // This I don't remember :D

    if (isSliding) {
        ApplyFrictionBetween(block, *surface);
    }
}

void SnapsEngine::ApplyFrictionBetween(Block& block, const Block& surface) {
    const float blockAcceleration = block.ForceAccum.x * block.InvMass;
    const float blockFinalVelocity = block.Velocity.x + blockAcceleration * m_DeltaTime;
    const float relativeVelocity = std::abs(blockFinalVelocity - surface.Velocity.x);
    if (relativeVelocity <= 0.01f) {
        block.Velocity.x = surface.Velocity.x;
        return;
    }

    const float dir = block.Velocity.x > 0 ? 1.0f : -1.0f;
    const float mass = 1.0f / block.InvMass;
    const float multiplier = std::sqrt(block.Friction * surface.Friction);

    // Assume the vertical force (gravity) is towards the surface
    float frictionForce = std::abs(block.ForceAccum.y) * multiplier * mass * dir;

    // Clamp: if this force would reverse velocity, zero it instead
    const float maxForce = mass * relativeVelocity / m_DeltaTime;
    if (std::abs(frictionForce) > maxForce) {
        frictionForce = maxForce * dir;
        std::cout << "friction stopped the object" << std::endl;
    }

    block.ForceAccum.x -= frictionForce;
}

void SnapsEngine::ApplyDrag(Block& block) {
    if (block.InvMass <= 0.0f) return;
    if (m_Config.Drag <= 0.0f) return;

    // If the block is almost stationary, skip to avoid tiny forces
    if (std::abs(block.Velocity.x) < 0.01f and std::abs(block.Velocity.y) < 0.01) return;

    Vector2 dragForce = block.Velocity * m_Config.Drag;

    const float blockAcceleration = block.ForceAccum.x * block.InvMass;
    const float blockFinalVelocity = block.Velocity.x + blockAcceleration * m_DeltaTime;
    const float mass = 1.0f / block.InvMass;
    const float maxForce = mass * blockFinalVelocity / m_DeltaTime;
    if (std::abs(dragForce.x) > std::abs(maxForce)) {
        dragForce.x = maxForce;
        std::cout << "drag stopped the object" << std::endl;
    }

    // Don't apply drag for slow objects to reduce snapping.
    if (std::abs(block.Velocity.x) < 1 / m_DeltaTime) {
        dragForce.x = 0;
    }
    if (std::abs(block.Velocity.y) < 1 / m_DeltaTime) {
        dragForce.y = 0;
    }

    block.ForceAccum -= dragForce;
}

// FIXME: Optimisation idea: instead of calculating finalX here and other things in applyGravity/Drag
//        I could apply just velocity (without acceleration) and then decide whether I should apply
//        resistance forces and how much. Only after that I would apply the forces by adding them to
//        velocity and then to position.
void SnapsEngine::DiscardResistanceForcesIfNecessary(int x, int y, Block& block) {
    const float minSlidingVelocity = m_Config.SmoothSnappingMinVelocity;
    const int alignedX = x * BLOCK_SIZE;
    const float distance = static_cast<float>(alignedX) - block.WorldPosition.x;
    if (std::abs(distance) == 0) return;

    // Calculate a final position as if deceleration was discarded and check
    // if a block will overshoot the current tile (reach the end)
    const float targetVelocityX = std::copysign(std::max(minSlidingVelocity, std::abs(block.Velocity.x)), block.Velocity.x);
    const float finalX = block.WorldPosition.x + targetVelocityX * m_DeltaTime;
    const bool movingRight = block.Velocity.x > 0;
    const float offset = movingRight ? BLOCK_SIZE : 0;
    const int finalXGrid = static_cast<int>(finalX + offset) / BLOCK_SIZE;
    if (finalXGrid != x) return;

    const float deceleration = -block.ForceAccum.x * block.InvMass;
    const float minVelocityToReachNextGrid = MinVelocityForDistance(deceleration, distance);

    // Block is too slow to reach the end of a tile assuming deceleration will be constant.
    if (std::abs(block.Velocity.x) < minVelocityToReachNextGrid) {
        // Discard deceleration force because if left it would stop the block mid-tile
        // not reaching the end of the tile. So we allow smooth sliding to the end with
        // no deceleration (friction, drag, et}c.)
        block.ForceAccum.x = 0;

        // If block is super slow, then we can speed it up
        if (std::abs(block.Velocity.x) < minSlidingVelocity) {
            block.Velocity.x = std::copysign(minSlidingVelocity, block.Velocity.x);
        }
    }
}

bool SnapsEngine::AreTouching(const Block& block1, const Block& block2) const {
    return block1.WorldPosition.x + BLOCK_SIZE >= block2.WorldPosition.x
        and block1.WorldPosition.x <= block2.WorldPosition.x + BLOCK_SIZE
        and block1.WorldPosition.y + BLOCK_SIZE >= block2.WorldPosition.y
        and block1.WorldPosition.y <= block2.WorldPosition.y + BLOCK_SIZE;
}

}
