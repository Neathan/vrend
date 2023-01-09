#pragma once

#include <GLFW/glfw3.h>

#include "graphics/renderer.h"


class App {
public:
	App(int width, int height)
		: m_width(width), m_height(height) {}

	~App();

	void init();
	void start();

private:
	int m_width, m_height;

	GLFWwindow *m_window = nullptr;

	Renderer m_renderer{};
};
