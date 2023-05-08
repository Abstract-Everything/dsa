#include "main_window.hpp"

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

int main(int argc, char const *argv[]) {
	spdlog::set_level(spdlog::level::trace);

	std::vector<std::string> arguments(static_cast<std::size_t>(argc));
	for (std::size_t i = 0; i < arguments.size(); ++i)
	{
		arguments[i] = argv[i];
	}

	auto &window = visual::Main_Window::instance();
	window.initialise(arguments);
	window.start();
	return 0;
}
