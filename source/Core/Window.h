#pragma once
#include <cstdint>

struct GLFWwindow;
class Window
{
public:
	Window(uint32_t width, uint32_t height, const char* title);
	~Window();

	bool IsRunning() const;
	void UpdateEvents();

	GLFWwindow* GetGLFWWindow() const { return m_window; }

private:
	GLFWwindow* m_window = nullptr;
};