#ifndef VISUAL_TEXT_WIDGET_HPP
#define VISUAL_TEXT_WIDGET_HPP

#include "widget.hpp"

namespace visual
{

class Text_Widget : public Widget
{
 public:
	Text_Widget();

	void set_text(std::string_view string);

 protected:
	[[nodiscard]] sf::Vector2f content_size() const override;

	void content_draw(sf::RenderTarget &target, sf::RenderStates states)
	    const override;

 private:
	sf::Text m_text;
};

} // namespace visual
#endif
