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

void CommandContext::DrawBegin(VkRenderingInfo& renderingInfo) const
{
	
	vkCmdBeginRendering(m_vkCommandBuffer, &renderingInfo);
}

void CommandContext::DrawEnd() const
{
	vkCmdEndRendering(m_vkCommandBuffer);
}

void CommandContext::TransitionBarrier(
	VkImage image, 
	VkImageLayout oldLayout, 
	VkImageLayout newLayout, 
	VkAccessFlags2 srcAccessMask, 
	VkAccessFlags2 dstAccessMask, 
	VkPipelineStageFlags2 srcStageMask, 
	VkPipelineStageFlags2 dstStageMask
) const
{
	VkImageMemoryBarrier2 barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrier.pNext = nullptr;
	barrier.image = image;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.srcStageMask = srcStageMask;
	barrier.dstStageMask = dstStageMask;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	VkDependencyInfo depedencyInfo{};
	depedencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depedencyInfo.pNext = nullptr;
	depedencyInfo.imageMemoryBarrierCount = 1;
	depedencyInfo.pImageMemoryBarriers = &barrier;

	vkCmdPipelineBarrier2(m_vkCommandBuffer, &depedencyInfo);
}

void CommandContext::ClearColorScreen(VkImageView view, float r, float g, float b, float a) const
{
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.pNext = nullptr;
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue.color = { r, g, b, a };
	colorAttachment.imageView = view;
	
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.pNext = nullptr;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;

	DrawBegin(renderingInfo);

	vkCmdBeginRendering(m_vkCommandBuffer, &renderingInfo);
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
