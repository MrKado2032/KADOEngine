#pragma once
#include <memory>

class Renderer;
class Window;
class Application
{
public:
	Application();
	virtual ~Application();

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	void Run();

protected:
	virtual void Start() = 0;
	virtual void Update(float dt) = 0;

private:
	std::unique_ptr<Window> m_window;
	std::unique_ptr<Renderer> m_renderer;
};

std::unique_ptr<Application> CreateApplication();