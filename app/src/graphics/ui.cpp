#include "ui.h"

#include <glad/vulkan.h>

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>


void initImGui(GLFWwindow *window, const Renderer &renderer) {
	//1: create descriptor pool for IMGUI
	// the size of the pool is very oversize, but it's copied from imgui demo itself.
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool imguiPool;
	vkCreateDescriptorPool(renderer.getDevice().getLogicalDevice(), &pool_info, nullptr, &imguiPool);


	// 2: initialize imgui library

	//this initializes the core structures of imgui
	ImGui::CreateContext();

	//this initializes imgui for SDL
	ImGui_ImplGlfw_InitForVulkan(window, true);

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = renderer.getInstance();
	init_info.PhysicalDevice = renderer.getDevice().getPhysicalDevice();
	init_info.Device = renderer.getDevice().getLogicalDevice();
	init_info.Queue = renderer.getDevice().getGraphicsQueue();
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, renderer.getRenderPass().getRenderPass());
	

	//execute a gpu command to upload imgui font textures
	VkCommandBuffer commandBuffer = renderer.prepareSingleCommand();
	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
	renderer.executeSingleCommand(commandBuffer);
	

	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();

// 	//add the destroy the imgui created structures
// 	_mainDeletionQueue.push_function([=]() {
// 
// 		vkDestroyDescriptorPool(_device, imguiPool, nullptr);
// 		ImGui_ImplVulkan_Shutdown();
// 		});
}

void uiBeginFrame() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void uiEndFrame(VkCommandBuffer commandBuffer) {
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
