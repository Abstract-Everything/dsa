#ifndef VISUAL_ARRAY_WIDGET_HPP
#define VISUAL_ARRAY_WIDGET_HPP

#include "text_widget.hpp"
#include "widget.hpp"

#include <memory>
#include <vector>

namespace visual
{

enum class Draw_Direction
{
	Horizontal,
	Vertical
};

template<typename T = Widget>
class Array_Widget : public Widget
{
 public:
	using Value          = T;
	using Widgets        = std::vector<std::unique_ptr<Value>>;
	using Iterator       = typename Widgets::iterator;
	using Const_Iterator = typename Widgets::const_iterator;

	explicit Array_Widget(Draw_Direction direction) : m_direction(direction)
	{
	}

	Array_Widget(std::size_t size, const Widget &widget, Draw_Direction direction)
	    : m_direction(direction)
	{
		m_widgets.clear();
		for (std::size_t i = 0; i < size; ++i)
		{
			m_widgets.push_back(
			    std::unique_ptr<Value>(widget.clone()));
		}
	}

	~Array_Widget() override = default;

	[[nodiscard]] Array_Widget<Value> *clone() const override
	{
		auto *array = new Array_Widget<Value>(m_direction);
		for (auto const &widget : m_widgets)
		{
			array->push_back(std::unique_ptr<Value>(widget->clone()));
		}
		return array;
	}

	void insert(Iterator index, std::unique_ptr<Value> value)
	{
		m_widgets.insert(index, std::move(value));
	}

	void push_back(std::unique_ptr<Value> value)
	{
		m_widgets.push_back(std::move(value));
	}

	void erase(Iterator index)
	{
		m_widgets.erase(index);
	}

	Iterator begin()
	{
		return m_widgets.begin();
	}

	Const_Iterator begin() const
	{
		return m_widgets.begin();
	}

	Iterator end()
	{
		return m_widgets.end();
	}

	Const_Iterator end() const
	{
		return m_widgets.end();
	}

 protected:
	[[nodiscard]] sf::Vector2f content_size() const override
	{
		sf::Vector2f size{ 0.0F, 0.0F };
		for (auto const &widget : m_widgets)
		{
			size.x += widget->size().x;
			size.y = std::max(size.y, widget->size().y);
		}
		return size;
	}

	void content_draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		for (auto const &widget : m_widgets)
		{
			target.draw(*widget, states);
			sf::Vector2f offset{0.0F, 0.0F};
			switch (m_direction)
			{
			case Draw_Direction::Horizontal:
				offset.x += widget->size().x;
				break;

			case Draw_Direction::Vertical:
				offset.y += widget->size().y;
				break;
			}
			states.transform.translate(offset);
		}
	}

 private:
	const Draw_Direction m_direction;

	Widgets m_widgets;
};

} // namespace visual

#endif
