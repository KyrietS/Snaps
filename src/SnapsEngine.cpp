#include "snaps/SnapsEngine.hpp"
#include "snaps/Block.hpp"
#include <raymath.h>
#include <iostream>
#include <cmath>


namespace snaps {

namespace {
bool TouchesFloor(const Grid& grid, const int x, const int y, const Block& block) {
    if (not grid.InBounds(x, y+1)) return true;
    const auto& below = grid.At(x, y+1);
    return below.has_value()
        and (block.WorldPosition.y + BLOCK_SIZE >= below->WorldPosition.y)
        and below->Velocity.x == 0
        and block.Velocity.y >= 0;
}
}

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
            SolveGridPhysics(x, y);
        }
        m_SecondPass = true;
        while (not m_RightMovementContacts.empty()) {
            auto [xx, yy] = m_RightMovementContacts.top();
            SolveGridPhysics(xx, yy);
            m_RightMovementContacts.pop();
        }
        m_SecondPass = false;
    }

    m_SecondPass = true;
    while (not m_UpMovementContacts.empty()) {
        auto [x, y] = m_UpMovementContacts.top();
        SolveGridPhysics(x, y);
        m_UpMovementContacts.pop();
    }
    m_SecondPass = false;
}

void SnapsEngine::SimulateMovement(const int x, const int y, Block& block) {
    if (block.IsDynamic) {
        ApplyGravity(block);
        if (TouchesFloor(m_Grid, x, y, block)) {
            ApplyFriction(block, 1.0f);
        } else {
            ApplyFriction(block, 0.0f); // drag
        }
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

void SnapsEngine::SolveGridPhysics(int x, int y) {
    auto& block = m_Grid.At(x, y);
    if (not block.has_value() or not block->IsDynamic or not block->NeedsCollisionResolution) return;
    SolveGridPhysics(x, y, *block);
}

void SnapsEngine::SolveGridPhysics(int x, int y, Block& block) {

    MovementResolution resolution {x, y};

    SolveMovementHorizontal(block, resolution);
    if (resolution.Resolved) return;

    assert(m_Grid.At(resolution.X, resolution.Y).has_value());
    auto& movedBlock = *m_Grid.At(resolution.X, resolution.Y);

    SolveMovementVertical(block, resolution);
    if (resolution.Resolved) return;

    // Mark as resolved so we don't try to resolve it again this frame.
    movedBlock.NeedsCollisionResolution = false;
}

void SnapsEngine::SolveMovementHorizontal(Block& block, MovementResolution& resolution) {
    if (block.Velocity.x >= 0)
        SolveMovementRight(block, resolution);
    else
        SolveMovementLeft(block, resolution);
}

void SnapsEngine::SolveMovementVertical(Block& block, MovementResolution& resolution) {
    if (block.Velocity.y >= 0)
        SolveMovementDown(block, resolution);
    else
        SolveMovementUp(block, resolution);
}

void SnapsEngine::SolveMovementLeft(Block& block, MovementResolution& resolution) {
    const int x = resolution.X;
    const int y = resolution.Y;

    // Block is not moving left
    if (block.Velocity.x >= 0) return; // moving right

    // No blocks to the left
    if (not m_Grid.InBounds(x-1, y)) { // at left edge of grid
        block.Velocity = {0, 0};
        return;
    }

    const int desiredXGrid = static_cast<int>(block.WorldPosition.x) / BLOCK_SIZE;
    auto& blockLeft = m_Grid.At(x - 1, y);
    const bool wantsToMoveLeft = desiredXGrid < x and block.Velocity.x < 0;
    const float deceleration = block.Acceleration.x > 0 ? block.Acceleration.x : 0.0f;

    // Desired grid is occupied. Stop.
    if (wantsToMoveLeft and blockLeft.has_value()) {
        block.WorldPosition.x = static_cast<float>(x) * BLOCK_SIZE;
        block.Velocity.x = 0;
        return;
    }

    // [DECELERATION ONLY]
    // Desired grid is free. Claim it if we have enough velocity to reach it.
    if (wantsToMoveLeft and not blockLeft.has_value()) {
        const float minVelocityToReachNextGrid = std::sqrt(2.0f * deceleration * BLOCK_SIZE);

        // Not enough velocity to reach the next grid. Stop and align to grid.
        if (std::abs(block.Velocity.x) < minVelocityToReachNextGrid) {
            block.WorldPosition.x = static_cast<float>(x) * BLOCK_SIZE;
            block.Velocity.x = 0;
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

    // No blocks to the right
    if (not m_Grid.InBounds(x+1, y)) { // at right edge of grid
        block.Velocity = {0, 0};
        return;
    }

    const int desiredXGrid = static_cast<int>(block.WorldPosition.x + BLOCK_SIZE) / BLOCK_SIZE;
    auto& blockRight = m_Grid.At(x + 1, y);
    const bool wantsToMoveRight = desiredXGrid > x and block.Velocity.x > 0;
    const float deceleration = block.Acceleration.x < 0 ? -block.Acceleration.x : 0.0f;

    // Desired grid is occupied. Stop.
    if (wantsToMoveRight and blockRight.has_value()) {
        if (blockRight->IsDynamic and not m_SecondPass) {
            // Try to solve in the second pass
            m_RightMovementContacts.push({x, y});
            resolution.Resolved = true;
        } else {
            block.WorldPosition.x = static_cast<float>(x) * BLOCK_SIZE;
            block.Velocity.x = 0;
        }
        return;
    }

    // [DECELERATION ONLY]
    // Desired grid is free. Claim it if we have enough velocity to reach it.
    if (wantsToMoveRight and not blockRight.has_value()) {
        const float minVelocityToReachNextGrid = std::sqrt(2.0f * deceleration * BLOCK_SIZE);

        // Not enough velocity to reach next grid. Stop and align to grid.
        if (std::abs(block.Velocity.x) < minVelocityToReachNextGrid) {
            block.WorldPosition.x = static_cast<float>(x) * BLOCK_SIZE;
            block.Velocity.x = 0;
            return;
        } else if (block.Velocity.x > 0) { // Claim grid to the right.
            blockRight = block;
            m_Grid.Remove(x, y);
            resolution.X += 1;
        }
    }

    // ALIGN POSITION TO GRID IF NECESSARY
    const int distanceFromCorrectPosition = static_cast<int>(static_cast<float>(x * BLOCK_SIZE) - block.WorldPosition.x);
    // Block stopped before reaching end of its own grid.
    if (block.Velocity.x == 0 and block.Acceleration.x == 0 and distanceFromCorrectPosition != 0) {
        std::cout << "block stopped without reaching end of its own grid\n";
        // Snap it to the grid in one shot.
        block.WorldPosition.x = static_cast<float>(x) * BLOCK_SIZE;
    }
}

void SnapsEngine::SolveMovementDown(Block& block, MovementResolution& resolution) {
    const int x = resolution.X;
    const int y = resolution.Y;
    // Block is not moving down
    if (block.Velocity.y < 0) return;

    // No block below
    if (not m_Grid.InBounds(x, y+1)) {
        block.Velocity = {0, 0};
        return;
    }

    const int desiredYGrid = static_cast<int>(block.WorldPosition.y + BLOCK_SIZE) / BLOCK_SIZE;
    auto& blockBelow = m_Grid.At(x, y+1);
    const bool wantsToMoveDown = desiredYGrid > y;

    // Desired grid is occupied. Stop.
    if (wantsToMoveDown and blockBelow.has_value()) {
        block.WorldPosition.y = static_cast<float>(y) * BLOCK_SIZE;
        block.Velocity.y = 0;
        return;
    }

    // [ACCELERATION ONLY]
    // Desired grid is free, claim it.
    if (wantsToMoveDown and not blockBelow.has_value()) {
        blockBelow = block;
        m_Grid.Remove(x, y);
        resolution.Y += 1;
        return;
    }

    // [ACCELERATION ONLY]
    // Only accelerated movements mid-air collision can result with a stop because
    // the gravity will make the block fall again to the desired spot.
    if (blockBelow and block.WorldPosition.y + BLOCK_SIZE >= blockBelow->WorldPosition.y) { // collided with block mid-fall
        block.Velocity.y = 0;
    }
}

void SnapsEngine::SolveMovementUp(Block& block, MovementResolution& resolution) {
    const int x = resolution.X;
    const int y = resolution.Y;
    // Block is not moving up
    if (block.Velocity.y >= 0) return; // moving down

    // No blocks above
    if (not m_Grid.InBounds(x, y-1)) { // at bottom of grid
        block.Velocity = {0, 0};
        return;
    }

    const int desiredYGrid = static_cast<int>(block.WorldPosition.y) / BLOCK_SIZE;
    auto& blockAbove = m_Grid.At(x, y - 1);
    const bool wantsToMoveUp = desiredYGrid < y;

    // Desired grid is occupied. Stop.
    if (wantsToMoveUp and blockAbove.has_value()) {
        if (blockAbove->IsDynamic and not m_SecondPass) {
            m_UpMovementContacts.push({x, y});
            resolution.Resolved = true;
        } else {
            block.WorldPosition.y = static_cast<float>(y) * BLOCK_SIZE;
            block.Velocity.y = 0;
        }
        return;
    }

    // [DECELERATION ONLY]
    // Desired grid is free. Claim it if we have enough velocity to reach it.
    if (wantsToMoveUp and not blockAbove.has_value()) {
        const float minVelocityToReachNextGrid = std::sqrt(2.0f * m_Config.Gravity * BLOCK_SIZE);
        // More accurate check if next grid is reachable
        // const float distanceToReachNextGrid = block.WorldPosition.y - desiredYGrid * BOX_SIZE;
        // const float minVelocityToReachNextGrid = std::sqrt(2.0f * GRAVITY * distanceToReachNextGrid);

        // Not enough velocity to reach next grid. Stop.
        if (std::abs(block.Velocity.y) < minVelocityToReachNextGrid) {
            block.WorldPosition.y = static_cast<float>(y) * BLOCK_SIZE;
            block.Velocity.y = 0;
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

void SnapsEngine::ApplyFriction(Block& block, float multiplier) {
    float speed = std::abs(block.Velocity.x) + block.ForceAccum.x * block.InvMass * m_DeltaTime;
    if (speed <= 1e-6f) {
        block.Velocity.x = 0;
        return;
    }

    float dir = block.Velocity.x > 0 ? 1.0f : -1.0f;
    float mass = 1.0f / block.InvMass;

    // Assume the gravity is already applied and it's directed downwards
    float frictionForce = std::abs(block.ForceAccum.y) * multiplier * mass * dir;

    // Clamp: if this force would reverse velocity, zero it instead
    float maxForce = mass * speed / m_DeltaTime;
    if (std::abs(frictionForce) > maxForce) {
        frictionForce = maxForce * dir;
        std::cout << "friction stopped the object" << std::endl;
    }

    block.ForceAccum.x -= frictionForce;
}

}
