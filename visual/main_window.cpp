#include "main_window.hpp"

#include "allocated_array_event.hpp"
#include "copy_assignment_event.hpp"
#include "element_monitor.hpp"
#include "memory_monitor.hpp"
#include "move_assignment_event.hpp"

#include <dsa/dynamic_array.hpp>

#include <SFML/Graphics.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#include <exception>

#include <imgui-SFML.h>

namespace fs = std::filesystem;

namespace
{
constexpr std::size_t x_resolution = 640;
constexpr std::size_t y_resolution = 640;
constexpr std::size_t frame_rate   = 60;
constexpr std::size_t array_size   = 5;
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
	ImGui::SFML::Init(m_window, true);
}

Main_Window::~Main_Window()
{
	ImGui::SFML::Shutdown();
}

const fs::path &Main_Window::executable_path()
{
	return m_executable;
}

const sf::Font &Main_Window::default_font()
{
	return m_font;
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

	fs::path times_path = fonts_path / "times-new-roman.ttf";
	m_font.loadFromFile(times_path.string());
}

void Main_Window::start()
{
	const sf::Color dark_grey{ 25, 25, 25 };

	dsa::Dynamic_Array<Element_Monitor<int>, Memory_Monitor<Element_Monitor<int>>>
	    array_a{ Memory_Monitor<Element_Monitor<int>>{} };

	array_a.resize(array_size);
	array_a[0] = Element_Monitor<int>{ 0 };
	array_a[1] = Element_Monitor<int>{ 1 };
	array_a[2] = Element_Monitor<int>{ 2 };

	array_a.resize(2 * array_size);

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

		process_events();

		ImGui::SFML::Update(m_window, deltaClock.restart());
		sf::RenderStates states;

		m_window.clear(dark_grey);
		m_viewport.draw(m_window, states);
		ImGui::SFML::Render(m_window);
		m_window.display();
	}
}

void Main_Window::add_event(std::unique_ptr<Event> event)
{
	m_events.push_back(std::move(event));
}

void Main_Window::process_events()
{
	for (auto const &event : m_events)
	{
		if (auto const *allocated_array =
			dynamic_cast<Allocated_Array_Event const *>(event.get()))
		{
			m_viewport.process(*allocated_array);
		}
		else if (
		    auto const *move_assignment =
			dynamic_cast<Move_Assignment_Event const *>(event.get()))
		{
			m_viewport.process(*move_assignment);
		}
		else if (
		    auto const *copy_assignment =
			dynamic_cast<Copy_Assignment_Event const *>(event.get()))
		{
			m_viewport.process(*copy_assignment);
		}
		else if (
		    auto const *deallocated_array =
			dynamic_cast<Deallocated_Array_Event const *>(event.get()))
		{
			m_viewport.process(*deallocated_array);
		}
		else
		{
			Event const *event_ptr = event.get();
			spdlog::error(
			    "Unhandled event of type {}",
			    typeid(event_ptr).name());
		}
	}
	m_events.clear();
}
} // namespace visual
