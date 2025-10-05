#pragma once

namespace Brick {

constexpr int BOX_SIZE = 16;
constexpr float GRAVITY = 200.0f;
constexpr float TIME_MULTIPLIER = 1.0f;


extern bool tick;

inline float DeltaTime() {
	// return 0.008f;
	// if (tick) {
	// 	return 0.016f; // simulate 60 FPS
	// }
	// return 0.0f;

	// ONLY FOR DEBUGGING PURPOSES
	return 0.016f;
	// float dt =  GetFrameTime() * TIME_MULTIPLIER;
	// if (dt > 0.016f) {
	// 	return 0.016f; // simulate 60 FPS
	// }
	// return dt;
}

}