#ifndef VISUAL_MAIN_WINDOW_HPP
#define VISUAL_MAIN_WINDOW_HPP

#include "viewport.hpp"

#include <SFML/Graphics.hpp>

#include <filesystem>

namespace visual
{

class User_Interface;

class Main_Window
{
 public:
	Main_Window(Main_Window const &) = delete;
	Main_Window(Main_Window &&)      = delete;

	Main_Window &operator=(Main_Window const &) = delete;
	Main_Window &operator=(Main_Window &&)      = delete;

	static Main_Window &instance();

	std::filesystem::path const &executable_path();

	static bool is_registering() {
		return instance().m_register_events;
	}

	static void registering(bool registering) {
		instance().m_register_events = registering;
	}

	static void add_event(dsa::Memory_Monitor_Event auto &&event) {
		instance().add_event_impl(std::forward<decltype(event)>(event));
	}

	void initialise(std::vector<std::string> const &arguments);
	void start();

 private:
	Main_Window();
	~Main_Window();

	std::filesystem::path m_executable;

	sf::RenderWindow m_window;

	std::atomic_bool m_register_events = true;
	Viewport         m_viewport;

	std::unique_ptr<User_Interface> m_user_interface;

	void add_event_impl(dsa::Memory_Monitor_Event auto &&event) {
		if (m_register_events)
		{
			m_viewport.add_event(std::forward<decltype(event)>(event));
		}
	}
};

} // namespace visual

#endif
