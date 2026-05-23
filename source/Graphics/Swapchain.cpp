#include "Swapchain.h"
#include <stdexcept>
#include <algorithm>
#include <spdlog/spdlog.h>

Swapchain::Swapchain(const SwapchainCreateInfo& createInfo) : m_device(createInfo.device)
{
	if (!Create_Swapchain(createInfo))
	{
		throw std::runtime_error("failed to create swap chain");
	}

	if (!Create_ImageViews(createInfo.device))
	{
		throw std::runtime_error("failed to create image views");
	}
}

Swapchain::~Swapchain()
{
	Destroy_ImageViews();

	Destroy_Swapchain();
}

bool Swapchain::AcquireNextImage(VkSemaphore signalSemaphore, uint32_t& imageIndex) const
{
	VkResult result = vkAcquireNextImageKHR(m_device, m_vkSwapchain, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		return false;
	}

	if ((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR))
	{
		return false;
	}
	
	return true;
}

bool Swapchain::Present(VkQueue queue, VkSemaphore waitSemaphore, uint32_t imageIndex)
{
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_vkSwapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &waitSemaphore;

	VkResult result = vkQueuePresentKHR(queue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		spdlog::error("Do recreate swap chain");
		return false;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		spdlog::error("failed to present");
		return false;
	}
	
	return true;
}

bool Swapchain::Create_Swapchain(const SwapchainCreateInfo& createInfo)
{
	VkSurfaceCapabilitiesKHR surface_caps{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(createInfo.phyDevice, createInfo.surface, &surface_caps);

	// 画面の縦横幅のクランプ処理
	if (surface_caps.currentExtent.width != UINT32_MAX)
	{
		m_extent = surface_caps.currentExtent;
	}
	else
	{
		m_extent.width = std::clamp(surface_caps.currentExtent.width, surface_caps.minImageExtent.width, surface_caps.maxImageExtent.width);
		m_extent.height = std::clamp(surface_caps.currentExtent.height, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height);
	}

	m_imageCount = surface_caps.minImageCount + 1;
	if (surface_caps.maxImageCount > 0 && m_imageCount > surface_caps.maxImageCount)
	{
		m_imageCount = surface_caps.maxImageCount;
	}

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(createInfo.phyDevice, createInfo.surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(createInfo.phyDevice, createInfo.surface, &formatCount, formats.data());

	m_format = formats[0];
	for (auto& format : formats)
	{
		if ((format.format == VK_FORMAT_B8G8R8A8_SRGB || format.format == VK_FORMAT_R8G8B8A8_SRGB) && 
			format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR
			)
		{
			m_format = format;
			break;
		}
	}

	VkPresentModeKHR presentMode;
	uint32_t presentCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(createInfo.phyDevice, createInfo.surface, &presentCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(createInfo.phyDevice, createInfo.surface, &presentCount, presentModes.data());

	presentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (auto& mode : presentModes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentMode = mode;
			break;
		}
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageColorSpace = m_format.colorSpace;
	swapchainCreateInfo.imageExtent = m_extent;
	swapchainCreateInfo.imageFormat = m_format.format;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.minImageCount = m_imageCount;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.preTransform = surface_caps.currentTransform;
	swapchainCreateInfo.surface = createInfo.surface;

	VkResult result = vkCreateSwapchainKHR(createInfo.device, &swapchainCreateInfo, nullptr, &m_vkSwapchain);
	if (result != VK_SUCCESS)
	{
		spdlog::error("failed to create swap chain");
		return false;
	}

	m_images.resize(m_imageCount);
	result = vkGetSwapchainImagesKHR(createInfo.device, m_vkSwapchain, &m_imageCount, m_images.data());
	if (result != VK_SUCCESS)
	{
		spdlog::error("failed to get swap chain images");
		return false;
	}

	return true;
}

bool Swapchain::Create_ImageViews(VkDevice device)
{
	m_imageViews.reserve(m_images.size());
	for (auto& image : m_images)
	{
		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.pNext = nullptr;
		viewCreateInfo.flags = 0;
		viewCreateInfo.format = m_format.format;
		viewCreateInfo.image = image;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;
		viewCreateInfo.subresourceRange.levelCount = 1;

		VkImageView view = VK_NULL_HANDLE;
		if (vkCreateImageView(device, &viewCreateInfo, nullptr, &view) != VK_SUCCESS)
		{
			spdlog::error("failed to create image view");
			return false;
		}

		m_imageViews.push_back(view);
	}
	
	return true;
}

void Swapchain::Destroy_Swapchain()
{
	if (m_vkSwapchain)
	{
		vkDestroySwapchainKHR(m_device, m_vkSwapchain, nullptr);
	}
}

void Swapchain::Destroy_ImageViews()
{
	if (!m_imageViews.empty())
	{
		for (auto& view : m_imageViews)
		{
			vkDestroyImageView(m_device, view, nullptr);
			view = VK_NULL_HANDLE;
		}
		m_imageViews.clear();
	}
}
