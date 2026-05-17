#include "Window.h"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

Window::Window(uint32_t width, uint32_t height, const char* title)
{
	if (!glfwInit())
	{
		spdlog::error("failed to initialize glfw");
		throw std::runtime_error("failed to initialize glfw");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	
	m_window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title, nullptr, nullptr);
	if (!m_window)
	{
		spdlog::error("failed to create window");
		glfwTerminate();
		throw std::runtime_error("failed to create window");
	}

	spdlog::info("Success Initialized Window");
}

Window::~Window()
{
	if (m_window)
	{
		glfwDestroyWindow(m_window);
		spdlog::info("Destroy glfw window");
	}

	glfwTerminate();
	spdlog::info("Terminated glfw");
}

bool Window::IsRunning() const
{
	return !glfwWindowShouldClose(m_window);
}

void Window::UpdateEvents()
{
	glfwPollEvents();
}
