#ifndef VISUAL_MAIN_WINDOW_HPP
#define VISUAL_MAIN_WINDOW_HPP

#include "event.hpp"
#include "viewport.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Font.hpp>

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
	const sf::Font              &default_font();

	void add_event(std::unique_ptr<Event> &&event);

	void initialise(const std::vector<std::string> &arguments);
	void start();

 private:
	Main_Window();
	~Main_Window();

	std::filesystem::path m_executable;

	sf::RenderWindow m_window;
	sf::Font         m_font;

	Viewport m_viewport;
};
} // namespace visual

#endif
