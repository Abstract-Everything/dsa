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
	Main_Window(const Main_Window &) = delete;
	Main_Window(Main_Window &&)      = delete;

	Main_Window &operator=(const Main_Window &) = delete;
	Main_Window &operator=(Main_Window &&) = delete;

	static Main_Window &instance();

	const std::filesystem::path &executable_path();

	static void add_event(dsa::Memory_Monitor_Event auto &&event)
	{
		instance().m_viewport.add_event(
		    std::forward<decltype(event)>(event));
	}

	void initialise(const std::vector<std::string> &arguments);
	void start();

 private:
	Main_Window();
	~Main_Window();

	std::filesystem::path m_executable;

	sf::RenderWindow m_window;

	Viewport       m_viewport;
	std::unique_ptr<User_Interface> m_user_interface;
};

} // namespace visual

#endif
