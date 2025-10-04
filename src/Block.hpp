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
};

inline void AddForce(Block& block, const Vector2 force) {
	block.ForceAccum += force;
}

inline void ApplyGravity(Block& block) {
	block.ForceAccum += Vector2{0.0f, GRAVITY} * block.GravityScale / block.InvMass;
}

inline void ApplyFriction(Block& block) {
	if (block.Velocity.x < 0.0f) {
		block.ForceAccum.x += FRICTION / block.InvMass;
	} else if (block.Velocity.x > 0.0f) {
		block.ForceAccum.x -= FRICTION / block.InvMass;
	}
}

inline void ApplyImpulse(Block& block, const Vector2 impulse) {
	block.Velocity += impulse * block.InvMass;
}

}