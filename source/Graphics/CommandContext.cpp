#include "CommandContext.h"
#include <stdexcept>
#include <spdlog/spdlog.h>

CommandContext::CommandContext(VkDevice device, uint32_t queueFamilyIndex) : m_vkDevice(device)
{
	Create_CommandPool(device, queueFamilyIndex);

	Allocate_CommandBuffer(device);
}

CommandContext::~CommandContext()
{
	if (m_vkCommandPool)
	{
		vkDestroyCommandPool(m_vkDevice, m_vkCommandPool, nullptr);
	}
}

bool CommandContext::Begin() const
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(m_vkCommandBuffer, &beginInfo) != VK_SUCCESS)
	{
		spdlog::error("failed to begin command buffer");
		return false;
	}

	return true;
}

bool CommandContext::End() const
{
	if (vkEndCommandBuffer(m_vkCommandBuffer) != VK_SUCCESS)
	{
		spdlog::error("failed to end command buffer");
		return false;
	}

	return true;
}

bool CommandContext::Reset() const
{
	if (vkResetCommandBuffer(m_vkCommandBuffer, 0) != VK_SUCCESS)
	{
		spdlog::error("failed to reset command buffer");
		return false;
	}

	return true;
}

bool CommandContext::Submit(VkQueue queue, VkSemaphore signalSemaphore, VkSemaphore waitSemaphore, VkFence fence) const
{
	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_vkCommandBuffer;
	submitInfo.pWaitDstStageMask = &waitDstStageMask;
	submitInfo.waitSemaphoreCount = waitSemaphore ? 1 : 0;
	submitInfo.pWaitSemaphores = &waitSemaphore;
	submitInfo.signalSemaphoreCount = signalSemaphore ? 1 : 0;
	submitInfo.pSignalSemaphores = &signalSemaphore;

	VkResult result = vkQueueSubmit(queue, 1, &submitInfo, fence);
	if (result != VK_SUCCESS)
	{
		spdlog::error("failed to submit queue");
		return false;
	}
	
	return true;
}

void CommandContext::Create_CommandPool(VkDevice device, uint32_t queueFamilyIndex)
{
	VkCommandPoolCreateInfo cmdPoolCreateInfo{};
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolCreateInfo.pNext = nullptr;
	cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

	VkResult result = vkCreateCommandPool(device, &cmdPoolCreateInfo, nullptr, &m_vkCommandPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool");
	}
}

void CommandContext::Allocate_CommandBuffer(VkDevice device)
{
	VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.pNext = nullptr;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandBufferCount = 1;
	cmdBufferAllocInfo.commandPool = m_vkCommandPool;

	VkResult result = vkAllocateCommandBuffers(device, &cmdBufferAllocInfo, &m_vkCommandBuffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffer");
	}
}
