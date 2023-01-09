#pragma once

#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include "appinfo.h"
#include "device.h"
#include "validation.h"
#include "extensions.h"


class Renderer {
public:
	~Renderer();

	void init(const AppInfo &info, GLFWwindow *window);

private:
	void configureDebugCallback(VkDebugUtilsMessengerCreateInfoEXT &debugCreateInfo);


	VkInstance m_instance{};
	VkSurfaceKHR m_surface{};

	Device m_device{};

	Validator m_validator{};
	Extensions m_instanceExtensions{};
	Extensions m_deviceExtensions{};

	VkDebugUtilsMessengerEXT m_debugMessenger{};
};
