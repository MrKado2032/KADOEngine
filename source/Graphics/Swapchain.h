#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>

struct SwapchainCreateInfo
{
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice phyDevice = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
};

class Swapchain
{
public:
	Swapchain(const SwapchainCreateInfo& createInfo);
	~Swapchain();

	Swapchain(const Swapchain&) = delete;
	Swapchain& operator=(const Swapchain&) = delete;

	bool AcquireNextImage(VkSemaphore signalSemaphore, uint32_t& imageIndex) const;
	bool Present(VkQueue queue, VkSemaphore waitSemaphore, uint32_t imageIndex);

private:
	bool Create_Swapchain(const SwapchainCreateInfo& createInfo);
	bool Create_ImageViews(VkDevice device);

	void Destroy_Swapchain();
	void Destroy_ImageViews();

	VkSwapchainKHR m_vkSwapchain = VK_NULL_HANDLE;
	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;

	VkExtent2D m_extent = {};
	VkSurfaceFormatKHR m_format = {};
	uint32_t m_imageCount = 0;

	VkDevice m_device = VK_NULL_HANDLE;
};