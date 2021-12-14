#ifndef VISUAL_MAIN_WINDOW_HPP
#define VISUAL_MAIN_WINDOW_HPP

#include "actions.hpp"
#include "event.hpp"
#include "user_interface.hpp"
#include "viewport.hpp"

#include <SFML/Graphics.hpp>

#include <filesystem>

namespace visual
{

class Main_Window
{
 public:
	Main_Window(const Main_Window &) = delete;
	Main_Window(Main_Window &&)      = delete;

	Main_Window &operator=(const Main_Window &) = delete;
	Main_Window &operator=(Main_Window &&) = delete;

	static Main_Window &instance();

	const std::filesystem::path &executable_path();

	void add_event(Event &&event);

	void initialise(const std::vector<std::string> &arguments);
	void start();

 private:
	Main_Window();
	~Main_Window();

	std::filesystem::path m_executable;

	sf::RenderWindow m_window;

	Viewport       m_viewport;
	User_Interface m_user_interface;
};

} // namespace visual

#endif
