#pragma once

#include <SFML/Graphics.hpp>

class Widget
{
public:
	Widget();

	void draw(sf::RenderWindow& renderTarget);
	void update(sf::RenderWindow& renderTarget, int mouseWheelDelta = 0);
	
	void setBorderRect(sf::FloatRect borderRect);
	void setRect(sf::FloatRect widgetRect);
	float getScale();
	void scale(float factor);
	void centerToTopLeft();

protected:
	virtual void draw_internal(sf::RenderWindow& renderTarget);
	virtual void draw_internalNoView(sf::RenderWindow& renderTarget);
	virtual void update_internal(sf::RenderWindow& renderTarget);
	void borderConstraintCheck();
	void drawSliders(sf::RenderWindow& renderTarget);

	sf::FloatRect m_sceneRectBorder;
	sf::FloatRect m_widgetRect;
	
	sf::View m_View;
	sf::Vector2f m_dragStartPos;
	bool m_dragged;

	sf::RectangleShape m_sliderBackgroundShape;
	sf::RectangleShape m_VsliderShape, m_HsliderShape;
	bool m_hSliderDragged, m_vSliderDragged;
	float m_sliderDragStartPos;
	sf::Vector2f m_centerPosSliderDragStart;
};

namespace WidgetUi {
	const sf::Color sliderColor = sf::Color(140, 140, 140);
	const sf::Color sliderBackgroundColor = sf::Color(200, 200, 200, 140);
	const float sliderThicness = 10;
	const float sliderLenOffset = 38;
	const float sliderBorderSpaceOffset = 12;
}
