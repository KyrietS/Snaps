#pragma once
#include <raylib.h>
#include "Constants.hpp"

namespace Brick {
struct Block {
	Vector2 WorldPosition = {0, 0};
	Vector2 Velocity = {0, 0};
	Color FillColor = PINK;
	bool IsDynamic = false;

	float InvMass = 1.0f;
	Vector2 ForceAccum = {0, 0};
	float LinearDamping = 0.0f;
	float GravityScale = 1.0f;

	// ----- Read only -----

	// Acceleration calculated each from ForceAccum. Do not set manually.
	Vector2 Acceleration = {0, 0};

	// Set during a physics step. All bodies that have moved need collision resolution.
	bool NeedsCollisionResolution = false;
};

inline void AddForce(Block& block, const Vector2 force) {
	block.ForceAccum += force;
}

inline void ApplyGravity(Block& block) {
	block.ForceAccum += Vector2{0.0f, GRAVITY} * block.GravityScale / block.InvMass;
}

inline void ApplyFriction(Block& block, float multiplier = 1.0f) {
	float speed = std::abs(block.Velocity.x);
	if (speed <= 1e-6f)
		return;

	float dir = block.Velocity.x > 0 ? 1.0f : -1.0f;
	float mass = 1.0f / block.InvMass;
	float frictionForce = FRICTION * multiplier * mass * dir;

	// Clamp: if this force would reverse velocity, zero it instead
	float maxForce = mass * speed / DeltaTime();
	if (std::abs(frictionForce) > maxForce) {
		frictionForce *= maxForce / std::abs(frictionForce);
	}

	block.ForceAccum.x -= frictionForce;
}

inline void ApplyImpulse(Block& block, const Vector2 impulse) {
	block.Velocity += impulse * block.InvMass;
}

}