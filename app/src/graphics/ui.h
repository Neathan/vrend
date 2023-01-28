#pragma once

#include <GLFW/glfw3.h>
#include <glad/vulkan.h>

#include "graphics/renderer.h"

void initImGui(GLFWwindow *window, const Renderer &renderer);
void destroyImGui();

void uiBeginFrame();
void uiEndFrame(VkCommandBuffer commandBuffer);