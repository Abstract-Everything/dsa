#include "main_window.hpp"

#include "event.hpp"

#include <dsa/dynamic_array.hpp>

#include <SFML/Graphics.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <exception>

namespace fs = std::filesystem;

namespace
{
constexpr std::size_t x_resolution = 640;
constexpr std::size_t y_resolution = 640;
constexpr std::size_t frame_rate   = 60;
} // namespace

namespace visual
{
Main_Window &Main_Window::instance()
{
	static Main_Window instance{};
	return instance;
}

Main_Window::Main_Window()
    : m_window{ sf::VideoMode(x_resolution, y_resolution),
		"Visualising Data Structures and Algorithms" }
{
	m_window.setFramerateLimit(frame_rate);
	ImGui::SFML::Init(m_window, false);
}

Main_Window::~Main_Window()
{
	ImGui::SFML::Shutdown();
}

const fs::path &Main_Window::executable_path()
{
	return m_executable;
}

void Main_Window::add_event(Event &&event)
{
	m_viewport.add_event(std::move(event));
}

void Main_Window::initialise(const std::vector<std::string> &arguments)
{
	constexpr std::string_view path_error =
	    "Unable to figure out the path to the executable";

	if (arguments.empty())
	{
		throw std::runtime_error(fmt::format(
		    "{}: ",
		    path_error,
		    "The executable path was not passed through argv"));
	}

	fs::path argument{ arguments[0] };
	if (argument.is_absolute())
	{
		m_executable = argument;
	}
	else
	{
		m_executable = fs::current_path() / arguments[0];
	}

	if (!fs::exists(m_executable) || fs::is_directory(m_executable))
	{
		throw std::runtime_error(fmt::format(
		    "{}: The assembled path is '{}'",
		    path_error,
		    m_executable));
	}

	fs::path fonts_path = m_executable.parent_path() / "../fonts";
	if (!fs::is_directory(fonts_path))
	{
		throw std::runtime_error(fmt::format(
		    "Expected the fonts directory to be located at '{}'",
		    fonts_path));
	}

	ImGuiIO io = ImGui::GetIO();
	fs::path times_path = fonts_path / "times-new-roman.ttf";
	ImFont* font = io.Fonts->AddFontFromFileTTF(times_path.string().c_str(), 40.0F);
	if (font == nullptr)
	{
		throw std::runtime_error(
		    fmt::format("Failed to load font in ImGui from '{}'", fonts_path));
	}
	font->Scale = 0.5F;
	ImGui::SFML::UpdateFontTexture();
}

void Main_Window::start()
{
	const sf::Color dark_grey{ 25, 25, 25 };

	sf::Clock deltaClock;
	while (m_window.isOpen())
	{
		sf::Event event{};
		while (m_window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed)
			{
				m_window.close();
			}
		}

		const sf::Time deltaTime = deltaClock.restart();
		ImGui::SFML::Update(m_window, deltaTime);
		m_viewport.update(
		    std::chrono::microseconds{ deltaTime.asMicroseconds() });

		m_window.clear(dark_grey);

		m_viewport.draw();

		bool             open  = true;
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
					 | ImGuiWindowFlags_NoMove
					 | ImGuiWindowFlags_NoResize
					 | ImGuiWindowFlags_NoSavedSettings;

		const ImGuiViewport *viewport = ImGui::GetMainViewport();
		const ImVec2         position = viewport->WorkPos;
		const ImVec2         size     = viewport->WorkSize;
		ImGui::SetNextWindowPos(
		    ImVec2{ position.x, position.y + size.y / 2 });
		ImGui::SetNextWindowSize(ImVec2{ size.x, size.y / 2 });

		if (ImGui::Begin("Data structure visualisation", &open, flags))
		{
			m_user_interface.draw();
		}
		ImGui::End();

		ImGui::SFML::Render(m_window);
		m_window.display();
	}
}

} // namespace visual
