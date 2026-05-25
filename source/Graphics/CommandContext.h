#pragma once
#include <vulkan/vulkan_core.h>

class CommandContext
{
public:
	CommandContext(VkDevice device, uint32_t queueFamilyIndex);
	~CommandContext();

	CommandContext(const CommandContext&) = delete;
	CommandContext& operator=(const CommandContext&) = delete;

	bool Begin() const;
	bool End() const;
	bool Reset() const;
	bool Submit(VkQueue queue, VkSemaphore signalSemaphore, VkSemaphore waitSemaphore, VkFence fence) const;

	void DrawBegin(VkRenderingInfo& renderingInfo) const;
	void DrawEnd() const;

	void TransitionBarrier(
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkAccessFlags2 srcAccessMask,
		VkAccessFlags2 dstAccessMask,
		VkPipelineStageFlags2 srcStageMask,
		VkPipelineStageFlags2 dstStageMask
	) const;

	void ClearColorScreen(VkImageView view, float r, float g, float b, float a = 1.f) const;

private:
	void Create_CommandPool(VkDevice device, uint32_t queueFamilyIndex);
	void Allocate_CommandBuffer(VkDevice device);

	VkCommandBuffer m_vkCommandBuffer = VK_NULL_HANDLE;
	VkCommandPool m_vkCommandPool = VK_NULL_HANDLE;

	VkDevice m_vkDevice = VK_NULL_HANDLE;
};