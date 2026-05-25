#include "Application.h"
#include <stdexcept>
#include <spdlog/spdlog.h>

#include "Core/Window.h"
#include "Graphics/GLFWSurfaceProvider.h"
#include "Graphics/GraphicsKernel.h"
#include "Graphics/Renderer.h"

Application::Application()
{
	m_window = std::make_unique<Window>(1280, 720, "KADOEngine");

	GLFWSurfaceProvider surfaceProvider(m_window->GetGLFWWindow());
	if (!GraphicsKernel::Initialize("KADOEngine", &surfaceProvider))
	{
		spdlog::error("failed to initialize graphics kernel");
		throw std::runtime_error("failed to initialize graphics kernel");
	}

	m_renderer = std::make_unique<Renderer>();

	spdlog::info("Success Initialized application");
}

Application::~Application()
{
	if (m_renderer)
	{
		m_renderer.reset();
	}
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

		m_renderer->BeginFrame();
		Update(1);
		m_renderer->EndFrame();
	}
}