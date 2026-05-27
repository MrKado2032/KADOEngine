#include "GraphicsKernel.h"
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include "Interface/ISurfaceProvider.h"

namespace GraphicsKernel
{
	static VkInstance s_vkInstance = VK_NULL_HANDLE;
	static VkDebugUtilsMessengerEXT s_vkDebugUtilsMessengerEXT = VK_NULL_HANDLE;
	static VkSurfaceKHR s_vkSurface = VK_NULL_HANDLE;
	static VkPhysicalDevice s_vkPhysicalDevice = VK_NULL_HANDLE;
	static VkDevice s_vkDevice = VK_NULL_HANDLE;
	static VkQueue s_GraphicsQueue = VK_NULL_HANDLE;
	static uint32_t s_GraphicsQueueIndex = 0;

	bool CreateInstance(const char* appName);
	bool CreateSurface(ISurfaceProvider* surfaceProvider);
	bool PickPhysicalDevice();
	bool CreateDevice();
	bool SetupDebugMessenger();
	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity, 
		VkDebugUtilsMessageTypeFlagsEXT type, 
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
		void*
	);

	bool Initialize(const char* appName, ISurfaceProvider* surfaceProvider)
	{
		if (!CreateInstance(appName))
		{
			return false;
		}

		if (!SetupDebugMessenger())
		{
			return false;
		}

		if (!CreateSurface(surfaceProvider))
		{
			return false;
		}

		if (!PickPhysicalDevice())
		{
			return false;
		}

		if (!CreateDevice())
		{
			return false;
		}

		spdlog::info("Initialized Graphics Kernel");
		return true;
	}

	void Destroy()
	{
		if (s_vkDevice)
		{
			vkDestroyDevice(s_vkDevice, nullptr);
		}

		if (s_vkSurface)
		{
			vkDestroySurfaceKHR(s_vkInstance, s_vkSurface, nullptr);
		}

		if (s_vkDebugUtilsMessengerEXT)
		{
			auto vkDestroyDebugUtilsMessengerEXT = 
				(PFN_vkDestroyDebugUtilsMessengerEXT)glfwGetInstanceProcAddress(s_vkInstance, "vkDestroyDebugUtilsMessengerEXT");

			vkDestroyDebugUtilsMessengerEXT(s_vkInstance, s_vkDebugUtilsMessengerEXT, nullptr);
		}

		if (s_vkInstance)
		{
			vkDestroyInstance(s_vkInstance, nullptr);
		}

		spdlog::info("Destroy Graphics Kernel");
	}

	VkInstance GetVkInstance()
	{
		return s_vkInstance;
	}

	VkPhysicalDevice GetVkPhysicalDevice()
	{
		return s_vkPhysicalDevice;
	}

	VkSurfaceKHR GetVkSurface()
	{
		return s_vkSurface;
	}

	VkDevice GetDevice()
	{
		return s_vkDevice;
	}

	VkQueue GetGraphicsQueue()
	{
		return s_GraphicsQueue;
	}

	uint32_t GetGraphicsQueueIndex()
	{
		return s_GraphicsQueueIndex;
	}

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(s_vkPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("Could not find suitable memory type");
	}

	bool CreateInstance(const char* appName)
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = appName;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
		appInfo.pEngineName = "KadoEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);

		uint32_t extensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

		std::vector<const char*> instanceExtensions(glfwExtensions, glfwExtensions + extensionCount);
		std::vector<const char*> instanceLayers;

		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = nullptr;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.flags = 0;
		instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
		instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();

		VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &s_vkInstance);
		if (result != VK_SUCCESS)
		{
			spdlog::error("failed to create instance");
			return false;
		}

		return true;
	}

	bool CreateSurface(ISurfaceProvider* surfaceProvider)
	{
		s_vkSurface = surfaceProvider->CreateSurface(s_vkInstance);
		if (!s_vkSurface)
		{
			spdlog::error("failed to create surface");
			return false;
		}

		return true;
	}

	bool PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(s_vkInstance, &deviceCount, nullptr);
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(s_vkInstance, &deviceCount, devices.data());

		for (auto& device : devices)
		{
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

			for (uint32_t i = 0; auto& queueProps : queueFamilyProperties)
			{
				if (queueProps.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					s_vkPhysicalDevice = device;
					s_GraphicsQueueIndex = i;
					return true;
				}

				i++;
			}
		}

		return false;
	}

	template<typename T>
	void BuildChainFeatures(T& last)
	{
		last.pNext = nullptr;
	}

	template<typename T, typename U, typename ...Rest>
	void BuildChainFeatures(T& current, U& next, Rest&...rest)
	{
		current.pNext = &next;
		BuildChainFeatures(next, rest...);
	}

	bool CreateDevice()
	{
		float priority = 1.0f;
		VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.pNext = nullptr;
		deviceQueueCreateInfo.flags = 0;
		deviceQueueCreateInfo.queueCount = 1;
		deviceQueueCreateInfo.queueFamilyIndex = s_GraphicsQueueIndex;
		deviceQueueCreateInfo.pQueuePriorities = &priority;

		VkPhysicalDeviceFeatures2 phyDeviceFeature2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		VkPhysicalDeviceVulkan11Features phyDeviceFeature11{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
		VkPhysicalDeviceVulkan12Features phyDeviceFeature12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		VkPhysicalDeviceVulkan13Features phyDeviceFeature13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		BuildChainFeatures(phyDeviceFeature2, phyDeviceFeature11, phyDeviceFeature12, phyDeviceFeature13);

		vkGetPhysicalDeviceFeatures2(s_vkPhysicalDevice, &phyDeviceFeature2);

		phyDeviceFeature13.dynamicRendering = VK_TRUE;
		phyDeviceFeature13.synchronization2 = VK_TRUE;

		std::vector<const char*> deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = &phyDeviceFeature2;
		deviceCreateInfo.pEnabledFeatures = nullptr;
		deviceCreateInfo.flags = 0;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		deviceCreateInfo.enabledLayerCount = 0;
		deviceCreateInfo.ppEnabledLayerNames = nullptr;

		VkResult result = vkCreateDevice(s_vkPhysicalDevice, &deviceCreateInfo, nullptr, &s_vkDevice);
		if (result != VK_SUCCESS)
		{
			spdlog::error("failed to create vk device");
			return false;
		}

		vkGetDeviceQueue(s_vkDevice, s_GraphicsQueueIndex, 0, &s_GraphicsQueue);

		return true;
	}

	bool SetupDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessageCreateInfo{};
		debugUtilsMessageCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugUtilsMessageCreateInfo.flags = 0;
		debugUtilsMessageCreateInfo.pNext = nullptr;
		debugUtilsMessageCreateInfo.messageSeverity = 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debugUtilsMessageCreateInfo.messageType = 
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debugUtilsMessageCreateInfo.pfnUserCallback = VulkanDebugCallback;

		auto vkCreateDebugUtilsMessengerEXT = 
			(PFN_vkCreateDebugUtilsMessengerEXT)glfwGetInstanceProcAddress(s_vkInstance, "vkCreateDebugUtilsMessengerEXT");

		if (vkCreateDebugUtilsMessengerEXT && 
			vkCreateDebugUtilsMessengerEXT(s_vkInstance, &debugUtilsMessageCreateInfo, nullptr, &s_vkDebugUtilsMessengerEXT) != VK_SUCCESS
			)
		{
			spdlog::error("failed to create debug utils messenger");
			return false;
		}

		return true;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity, 
		VkDebugUtilsMessageTypeFlagsEXT type, 
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
		void*
	)
	{
		spdlog::error("[Vulkan Layer] {}", pCallbackData->pMessage);
		return VK_FALSE;
	}
}