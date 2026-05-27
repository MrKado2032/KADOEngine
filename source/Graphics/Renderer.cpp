#include "Renderer.h"
#include <spdlog/spdlog.h>

#include "../Core/Window.h"
#include "GLFWSurfaceProvider.h"
#include "GraphicsKernel.h"
#include "Swapchain.h"
#include "CommandContext.h"

Renderer::Renderer(const Window& window)
{
	GLFWSurfaceProvider surfaceProvider(window.GetGLFWWindow());
	if (!GraphicsKernel::Initialize("KADOEngine", &surfaceProvider))
	{
		spdlog::error("failed to initialize graphics kernel");
		throw std::runtime_error("failed to initialize graphics kernel");
	}
	
	Create_CommandContext();

	Create_Swapchain();

	if (!Create_SyncObjects())
	{
		throw std::runtime_error("failed to create sync objects");
	}

	spdlog::info("Initialized Renderer");
}

Renderer::~Renderer()
{
	vkDeviceWaitIdle(GraphicsKernel::GetDevice());

	Destroy_SyncObjects();
	Destroy_Swapchain();
	Destroy_CommandContext();

	GraphicsKernel::Destroy();

	spdlog::info("Destroy Renderer");
}

void Renderer::BeginFrame()
{
	const auto device = GraphicsKernel::GetDevice();

	auto& frame = m_frames[m_currentFrameIndex];

	vkWaitForFences(device, 1, &frame.fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &frame.fence);

	frame.commandContext->Reset();

	frame.commandContext->Begin();

	const bool bResult = m_swapchain->AcquireNextImage(frame.imageAvailableSemaphore, m_imageIndex);
	if (!bResult)
	{

	}

	frame.commandContext->TransitionBarrier(
		m_swapchain->GetVkImageFromImageIndex(m_imageIndex),
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
		VK_ACCESS_2_NONE, 
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 
		VK_PIPELINE_STAGE_2_NONE, 
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
	);

	frame.commandContext->DrawBegin(m_swapchain->GetVkImageViewFromImageIndex(m_imageIndex));
}

void Renderer::EndFrame()
{
	const auto device = GraphicsKernel::GetDevice();

	auto& frame = m_frames[m_currentFrameIndex];

	frame.commandContext->DrawEnd();

	frame.commandContext->TransitionBarrier(
		m_swapchain->GetVkImageFromImageIndex(m_imageIndex),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_2_NONE,
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_2_NONE
	);

	frame.commandContext->End();

	bool bResult = frame.commandContext->Submit(GraphicsKernel::GetGraphicsQueue(), m_renderCompleteSemaphore[m_currentFrameIndex], frame.imageAvailableSemaphore, frame.fence);
	if (!bResult)
	{
		spdlog::error("failed to submit");
	}

	bResult = m_swapchain->Present(GraphicsKernel::GetGraphicsQueue(), m_renderCompleteSemaphore[m_currentFrameIndex], m_imageIndex);
	if (!bResult)
	{

	}

	m_currentFrameIndex = (m_currentFrameIndex + 1) % kFrameCount;
}

void Renderer::Create_CommandContext()
{
	const auto device = GraphicsKernel::GetDevice();
	
	for (auto& frame : m_frames)
	{
		frame.commandContext = std::make_unique<CommandContext>(device, GraphicsKernel::GetGraphicsQueueIndex());
	}
}

void Renderer::Create_Swapchain()
{
	const auto device = GraphicsKernel::GetDevice();
	const auto phyDevice = GraphicsKernel::GetVkPhysicalDevice();
	const auto surface = GraphicsKernel::GetVkSurface();

	SwapchainCreateInfo swapchainCreateInfo{};
	swapchainCreateInfo.device = device;
	swapchainCreateInfo.phyDevice = phyDevice;
	swapchainCreateInfo.surface = surface;
	
	m_swapchain = std::make_unique<Swapchain>(swapchainCreateInfo);
}

bool Renderer::Create_SyncObjects()
{
	const auto device = GraphicsKernel::GetDevice();

	VkSemaphoreCreateInfo semCreateInfo{};
	semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semCreateInfo.pNext = nullptr;

	for (auto& frame : m_frames)
	{	
		VkResult result = vkCreateSemaphore(device, &semCreateInfo, nullptr, &frame.imageAvailableSemaphore);
		if (result != VK_SUCCESS)
		{
			spdlog::error("failed to create image available semaphore");
			return false;
		}

		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		result = vkCreateFence(device, &fenceCreateInfo, nullptr, &frame.fence);
		if (result != VK_SUCCESS)
		{
			spdlog::error("failed to create fence");
			return false;
		}
	}

	m_renderCompleteSemaphore.resize(m_swapchain->GetImageCount());
	for (auto& sem : m_renderCompleteSemaphore)
	{
		VkResult result = vkCreateSemaphore(device, &semCreateInfo, nullptr, &sem);
		if (result != VK_SUCCESS)
		{
			spdlog::error("failed to create image available semaphore");
			return false;
		}
	}

	return true;
}

void Renderer::Destroy_CommandContext()
{
	for (auto& frame : m_frames)
	{
		frame.commandContext.reset();
		frame.commandContext = nullptr;
	}
}

void Renderer::Destroy_Swapchain()
{
	m_swapchain.reset();
	m_swapchain = nullptr;
}

void Renderer::Destroy_SyncObjects()
{
	const auto device = GraphicsKernel::GetDevice();
	
	for (auto& frame : m_frames)
	{
		vkDestroyFence(device, frame.fence, nullptr);
		vkDestroySemaphore(device, frame.imageAvailableSemaphore, nullptr);
	}

	for (auto& sem : m_renderCompleteSemaphore)
	{
		vkDestroySemaphore(device, sem, nullptr);
	}
}
