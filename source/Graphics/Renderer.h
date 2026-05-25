#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <array>
#include <memory>

class Swapchain;
class CommandContext;
class Renderer
{
public:
	static constexpr uint32_t kFrameCount = 2;

	Renderer();
	~Renderer();

	void BeginFrame();
	void EndFrame();

private:
	void Create_CommandContext();
	void Create_Swapchain();
	bool Create_SyncObjects();

	void Destroy_CommandContext();
	void Destroy_Swapchain();
	void Destroy_SyncObjects();

	struct FrameResource
	{
		std::unique_ptr<CommandContext> commandContext;
		VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
		VkFence fence = VK_NULL_HANDLE;
	};

	std::unique_ptr<Swapchain> m_swapchain;

	std::array<FrameResource, static_cast<size_t>(kFrameCount)> m_frames;
	std::vector<VkSemaphore> m_renderCompleteSemaphore;

	uint32_t m_imageIndex = 0;
	uint32_t m_currentFrameIndex = 0;

};