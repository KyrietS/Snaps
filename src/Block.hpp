#pragma once
#include <raylib.h>

namespace Brick {
struct Block {
	Vector2 WorldPosition = {0, 0};
	Vector2 Velocity = {0, 0};
	Color FillColor = PINK;
	bool IsDynamic = false;
};
}