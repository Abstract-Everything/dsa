#include "main_window.hpp"

#include <imgui.h>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <string>
#include <vector>

#include <imgui-SFML.h>

int main(int argc, const char *argv[])
{
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
