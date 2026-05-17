#pragma once
#include <iostream>
#include <memory>

extern std::unique_ptr<Application> CreateApplication();

int main()
{
	try {
		auto app = CreateApplication();
		app->Run();
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}