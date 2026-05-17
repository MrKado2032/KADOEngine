#include "GLFWSurfaceProvider.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

GLFWSurfaceProvider::GLFWSurfaceProvider(GLFWwindow* window)
{
	m_window = window;
}

VkSurfaceKHR GLFWSurfaceProvider::CreateSurface(VkInstance instance)
{
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkResult result = glfwCreateWindowSurface(instance, m_window, nullptr, &surface);
	if (result != VK_SUCCESS)
	{
		spdlog::error("failed to create glfw surface");
		return VK_NULL_HANDLE;
	}

	return surface;
}

uint32_t GLFWSurfaceProvider::GetFramebufferWidth() const
{
	int width;
	glfwGetFramebufferSize(m_window, &width, nullptr);

	return static_cast<uint32_t>(width);
}

uint32_t GLFWSurfaceProvider::GetFramebufferHeight() const
{
	int height;
	glfwGetFramebufferSize(m_window, nullptr, &height);

	return static_cast<uint32_t>(height);
}
