#pragma once
#include <raylib.h>
#include <raymath.h>
#include <iostream>

namespace snaps {

constexpr int BLOCK_SIZE = 16;

struct Block {
    Vector2 WorldPosition = {0, 0};
    Vector2 Velocity = {0, 0};
    Color FillColor = PINK;
    bool IsDynamic = false;

    float InvMass = 0.5f;
    Vector2 ForceAccum = {0, 0};
    float Friction = 1.0f;
    float GravityScale = 1.0f;

    // ----- Read only -----

    // Acceleration calculated after each step from ForceAccum. Do not set manually.
    Vector2 Acceleration = {0, 0};

    // Set during a physics step. All bodies that have moved need collision resolution.
    bool NeedsCollisionResolution = false;
};

/**
 * Adds force to be applied for the next simulation step. If you want to achieve constant force
 * you must call this function on every physics update
 */
inline void AddForce(Block& block, const Vector2 force) {
    block.ForceAccum += force;
}

/**
 * Applies an impulse force to the block, changing its velocity immediately.
 * The impulse is scaled by the block's mass so that heavier blocks need stronger impulse to move.
 */
inline void ApplyImpulse(Block& block, const Vector2 impulse) {
    block.Velocity += impulse * block.InvMass;
}

}