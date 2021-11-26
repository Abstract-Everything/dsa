#ifndef VISUAL_ARRAY_WIDGET_HPP
#define VISUAL_ARRAY_WIDGET_HPP

#include "element_widget.hpp"

#include <SFML/Graphics.hpp>

#include <vector>

namespace visual
{
class Array_Widget : public sf::Drawable
{
 public:
	explicit Array_Widget(
	    std::uint64_t address,
	    std::size_t   element_size,
	    std::size_t   size);

	[[nodiscard]] std::uint64_t address() const;

	[[nodiscard]] bool contains(std::uint64_t address) const;

	void on_assignment(
	    bool             initialised,
	    std::uint64_t    address,
	    std::string_view value);

	void invalidate_element(std::uint64_t address);
	void update_element(std::uint64_t address, std::string_view value);

	void resize(std::size_t size);

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

 private:
	std::uint64_t m_address{ 0 };
	std::size_t   m_element_size{ 0 };

	std::vector<Element_Widget> m_elements{};

	[[nodiscard]] std::uint64_t index_of(std::uint64_t address) const;
};

} // namespace visual

#endif
