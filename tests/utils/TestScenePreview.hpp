#pragma once
#include "TestScene.hpp"

class TestScenePreview {
public:
	TestScenePreview(const TestScene& scene);

	void Show();

private:
	const TestScene& m_Scene;
};
