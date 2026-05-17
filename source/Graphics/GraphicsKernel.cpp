#include "GraphicsKernel.h"
#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include "Interface/ISurfaceProvider.h"

namespace GraphicsKernel
{
	VkInstance s_vkInstance = VK_NULL_HANDLE;

	bool Initialize(const char* appName, ISurfaceProvider* surfaceProvider)
	{
		spdlog::info("Initialized Graphics Kernel");
		return true;
	}

	void Destroy()
	{
		spdlog::info("Destroy Graphics Kernel");
	}

	VkInstance GetVkInstance()
	{
		return s_vkInstance;
	}
}