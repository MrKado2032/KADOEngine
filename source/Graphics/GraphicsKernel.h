#pragma once
#include <vulkan/vulkan_core.h>

class ISurfaceProvider;
namespace GraphicsKernel
{
	bool Initialize(const char* appName, ISurfaceProvider* surfaceProvider);
	void Destroy();

	VkInstance GetVkInstance();
	VkPhysicalDevice GetVkPhysicalDevice();
	VkSurfaceKHR GetVkSurface();
	VkDevice GetDevice();
	VkQueue GetGraphicsQueue();
	uint32_t GetGraphicsQueueIndex();
}