#include "KADOEngine.h"

class SandboxApp : public Application
{
public:
	SandboxApp()
	{

	}

	~SandboxApp()
	{

	}

protected:
	void Start() override
	{

	}

	void Update(float dt) override
	{

	}
};

std::unique_ptr<Application> CreateApplication()
{
	return std::make_unique<SandboxApp>();
}