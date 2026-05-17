#include "Application.h"
#include <stdexcept>
#include <spdlog/spdlog.h>

#include "Core/Window.h"
#include "Graphics/GLFWSurfaceProvider.h"
#include "Graphics/GraphicsKernel.h"

Application::Application()
{
	m_window = std::make_unique<Window>(1280, 720, "KADOEngine");

	GLFWSurfaceProvider surfaceProvider(m_window->GetGLFWWindow());
	if (!GraphicsKernel::Initialize("KADOEngine", &surfaceProvider))
	{
		spdlog::error("failed to initialize graphics kernel");
		throw std::runtime_error("failed to initialize graphics kernel");
	}

	spdlog::info("Success Initialized application");
}

Application::~Application()
{
	GraphicsKernel::Destroy();

	if (m_window)
	{
		m_window.reset();
	}

	spdlog::info("Destroy application");
}

void Application::Run()
{
	Start();
	while (m_window->IsRunning())
	{
		m_window->UpdateEvents();
		Update(1);
	}
}