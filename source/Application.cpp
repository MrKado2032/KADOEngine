#include "Application.h"
#include <stdexcept>
#include <spdlog/spdlog.h>

#include "Core/Window.h"
#include "Graphics/Renderer.h"

Application::Application()
{
	m_window = std::make_unique<Window>(1280, 720, "KADOEngine");

	m_renderer = std::make_unique<Renderer>(*m_window);

	spdlog::info("Success Initialized application");
}

Application::~Application()
{
	if (m_renderer)
	{
		m_renderer.reset();
	}

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