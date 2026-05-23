#pragma once
#include <vulkan/vulkan_core.h>

class CommandContext
{
public:
	CommandContext(VkDevice device, uint32_t queueFamilyIndex);
	~CommandContext();

	bool Begin() const;
	bool End() const;
	bool Reset() const;
	bool Submit(VkQueue queue, VkSemaphore signalSemaphore, VkSemaphore waitSemaphore, VkFence fence) const;

private:
	void Create_CommandPool(VkDevice device, uint32_t queueFamilyIndex);
	void Allocate_CommandBuffer(VkDevice device);

	VkCommandBuffer m_vkCommandBuffer = VK_NULL_HANDLE;
	VkCommandPool m_vkCommandPool = VK_NULL_HANDLE;

	VkDevice m_vkDevice = VK_NULL_HANDLE;
};