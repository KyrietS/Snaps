#include "snaps/SnapsEngine.hpp"
#include "snaps/Block.hpp"
#include "snaps/Constants.hpp"
#include <raymath.h>
#include <iostream>



namespace Snaps {

namespace {
bool TouchesFloor(const Grid& grid, const int x, const int y, const Block& block) {
    if (not grid.InBounds(x, y+1)) return true;
    const auto& below = grid[x, y+1];
    return below.has_value() and (block.WorldPosition.y + BOX_SIZE >= below->WorldPosition.y) and below->Velocity.x == 0;
}
}

SnapsEngine::SnapsEngine(Grid& grid) : m_Grid(grid) {}

void SnapsEngine::Step(float deltaTime) {
    m_DeltaTime = deltaTime;
    SimulatePhysics();
}

void SnapsEngine::SimulatePhysics() {
    for (int y = 0; y < m_Grid.Height(); y++) {
        for (int x = 0; x < m_Grid.Width(); x++) {
            auto& block = m_Grid[x, y];
            if (block.has_value() and block->IsDynamic) {
                ApplyGravity(*block);
                if (TouchesFloor(m_Grid, x, y, *block)) {
                    ApplyFriction(*block);
                } else {
                    ApplyFriction(*block, 0.0f); // drag
                }
                Integrate(*block);
            }
        }
    }

    for (int y = m_Grid.Height() - 1; y >= 0; y--) {
        for (int x = 0; x < m_Grid.Width(); x++) {
            SolveGridPhysics(x, y);
        }
        m_SecondPass = true;
        while (not m_RightMovementContacts.empty()) {
            auto [x, y] = m_RightMovementContacts.top();
            SolveGridPhysics(x, y);
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

void SnapsEngine::Integrate(Block& block) {
    if (not block.IsDynamic or block.InvMass <= 0.0f) return;

    block.Acceleration = {
        block.ForceAccum.x * block.InvMass,
        block.ForceAccum.y * block.InvMass
    };

    block.Velocity += block.Acceleration * DeltaTime();

    // Realistically, any velocity smaller than 1.0/DeltaTime() will not move the object in a pixel space.
    if (std::abs(block.Velocity.x) < 0.01f) block.Velocity.x = 0.0f;
    if (std::abs(block.Velocity.y) < 0.01f) block.Velocity.y = 0.0f;

    block.WorldPosition += block.Velocity * DeltaTime();
    block.ForceAccum = {0, 0};

    block.NeedsCollisionResolution = true;
}

void SnapsEngine::SolveGridPhysics(int x, int y) {
    auto& block = m_Grid[x, y];
    if (not block.has_value() or not block->IsDynamic or not block->NeedsCollisionResolution) return;

    if (block->Velocity.x >= 0)
        SolveMovementRight(x, y, *block);
    else
        SolveMovementLeft(x, y, *block);

    if (x == -999) return; // will continue in second pass

    auto& movedBlock = m_Grid[x, y].value();

    if (movedBlock.Velocity.y >= 0)
        SolveMovementDown(x, y, movedBlock);
    else
        SolveMovementUp(x, y, movedBlock);

    if (x == -999) return; // will continue in second pass

    // Mark as resolved so we don't try to resolve it again this frame.
    movedBlock.NeedsCollisionResolution = false;
}


void SnapsEngine::SolveMovementLeft(int& x, int& y, Block& block) {
    // Block is not moving left
    if (block.Velocity.x >= 0) return; // moving right

    // No blocks to the left
    if (not m_Grid.InBounds(x-1, y)) { // at left edge of grid
        block.Velocity = {0, 0};
        return;
    }

    const int desiredXGrid = static_cast<int>(block.WorldPosition.x) / BOX_SIZE;
    auto& blockLeft = m_Grid[x - 1, y];
    const bool wantsToMoveLeft = desiredXGrid < x and block.Velocity.x < 0;
    const float deceleration = block.Acceleration.x > 0 ? block.Acceleration.x : 0.0f;

    // Desired grid is occupied. Stop.
    if (wantsToMoveLeft and blockLeft.has_value()) {
        block.WorldPosition.x = static_cast<float>(x) * BOX_SIZE;
        block.Velocity.x = 0;
        return;
    }

    // [DECELERATION ONLY]
    // Desired grid is free. Claim it if we have enough velocity to reach it.
    if (wantsToMoveLeft and not blockLeft.has_value()) {
        const float minVelocityToReachNextGrid = std::sqrtf(2.0f * deceleration * BOX_SIZE);

        // Not enough velocity to reach the next grid. Stop and align to grid.
        if (std::abs(block.Velocity.x) < minVelocityToReachNextGrid) {
            block.WorldPosition.x = static_cast<float>(x) * BOX_SIZE;
            block.Velocity.x = 0;
            return;
        } else { // Claim grid to the left.
            blockLeft = block;
            m_Grid.Remove(x, y);
            x -= 1;
        }
    }
}


void SnapsEngine::SolveMovementRight(int& x, int& y, Block& block) {
    // Block is not moving right
    if (block.Velocity.x < 0) return; // moving left

    // No blocks to the right
    if (not m_Grid.InBounds(x+1, y)) { // at right edge of grid
        block.Velocity = {0, 0};
        return;
    }

    const int desiredXGrid = static_cast<int>(block.WorldPosition.x + BOX_SIZE) / BOX_SIZE;
    auto& blockRight = m_Grid[x + 1, y];
    const bool wantsToMoveRight = desiredXGrid > x and block.Velocity.x > 0;
    const float deceleration = block.Acceleration.x < 0 ? -block.Acceleration.x : 0.0f;

    // Desired grid is occupied. Stop.
    if (wantsToMoveRight and blockRight.has_value()) {
        if (blockRight->IsDynamic and not m_SecondPass) {
            // Try to solve in the second pass
            m_RightMovementContacts.push({x, y});
            x = -999;
        } else {
            block.WorldPosition.x = static_cast<float>(x) * BOX_SIZE;
            block.Velocity.x = 0;
        }
        return;
    }

    // [DECELERATION ONLY]
    // Desired grid is free. Claim it if we have enough velocity to reach it.
    if (wantsToMoveRight and not blockRight.has_value()) {
        const float minVelocityToReachNextGrid = std::sqrtf(2.0f * deceleration * BOX_SIZE);

        // Not enough velocity to reach next grid. Stop and align to grid.
        if (std::abs(block.Velocity.x) < minVelocityToReachNextGrid) {
            block.WorldPosition.x = static_cast<float>(x) * BOX_SIZE;
            block.Velocity.x = 0;
            return;
        } else if (block.Velocity.x > 0) { // Claim grid to the right.
            blockRight = block;
            m_Grid.Remove(x, y);
            x += 1;
        }
    }

    // ALIGN POSITION TO GRID IF NECESSARY
    const int distanceFromCorrectPosition = x * BOX_SIZE - block.WorldPosition.x;
    // Block stopped before reaching end of its own grid.
    if (block.Velocity.x == 0 and block.Acceleration.x == 0 and distanceFromCorrectPosition != 0) {
        std::cout << "block stopped without reaching end of its own grid\n";
        // Snap it to the grid in one shot.
        block.WorldPosition.x = static_cast<float>(x) * BOX_SIZE;
    }
}

void SnapsEngine::SolveMovementDown(int& x, int& y, Block& block) {
    // Block is not moving down
    if (block.Velocity.y < 0) return;

    // No block below
    if (not m_Grid.InBounds(x, y+1)) {
        block.Velocity = {0, 0};
        return;
    }

    const int desiredYGrid = static_cast<int>(block.WorldPosition.y + BOX_SIZE) / BOX_SIZE;
    auto& blockBelow = m_Grid[x, y+1];
    const bool wantsToMoveDown = desiredYGrid > y;

    // Desired grid is occupied. Stop.
    if (wantsToMoveDown and blockBelow.has_value()) {
        block.WorldPosition.y = static_cast<float>(y) * BOX_SIZE;
        block.Velocity.y = 0;
        return;
    }

    // [ACCELERATION ONLY]
    // Desired grid is free, claim it.
    if (wantsToMoveDown and not blockBelow.has_value()) {
        blockBelow = block;
        m_Grid.Remove(x, y);
        y += 1;
        return;
    }

    // [ACCELERATION ONLY]
    // Only accelerated movements mid-air collision can result with a stop because
    // the gravity will make the block fall again to the desired spot.
    if (blockBelow and block.WorldPosition.y + BOX_SIZE >= blockBelow->WorldPosition.y) { // collided with block mid-fall
        block.Velocity.y = 0;
    }
}

void SnapsEngine::SolveMovementUp(int& x, int& y, Block& block) {
    // Block is not moving up
    if (block.Velocity.y >= 0) return; // moving down

    // No blocks above
    if (not m_Grid.InBounds(x, y-1)) { // at bottom of grid
        block.Velocity = {0, 0};
        return;
    }

    const int desiredYGrid = static_cast<int>(block.WorldPosition.y) / BOX_SIZE;
    auto& blockAbove = m_Grid[x, y - 1];
    const bool wantsToMoveUp = desiredYGrid < y;

    // Desired grid is occupied. Stop.
    if (wantsToMoveUp and blockAbove.has_value()) {
        if (blockAbove->IsDynamic and not m_SecondPass) {
            m_UpMovementContacts.push({x, y});
            x = -999;
        } else {
            block.WorldPosition.y = static_cast<float>(y) * BOX_SIZE;
            block.Velocity.y = 0;
        }
        return;
    }

    // [DECELERATION ONLY]
    // Desired grid is free. Claim it if we have enough velocity to reach it.
    if (wantsToMoveUp and not blockAbove.has_value()) {
        const float minVelocityToReachNextGrid = std::sqrtf(2.0f * GRAVITY * BOX_SIZE);
        // More accurate check if next grid is reachable
        // const float distanceToReachNextGrid = block.WorldPosition.y - desiredYGrid * BOX_SIZE;
        // const float minVelocityToReachNextGrid = std::sqrtf(2.0f * GRAVITY * distanceToReachNextGrid);

        // Not enough velocity to reach next grid. Stop.
        if (std::abs(block.Velocity.y) < minVelocityToReachNextGrid) {
            block.WorldPosition.y = static_cast<float>(y) * BOX_SIZE;
            block.Velocity.y = 0;
        } else { // Claim grid above.
            blockAbove = block;
            m_Grid.Remove(x, y);
            y -= 1;
        }
    }
}

}
