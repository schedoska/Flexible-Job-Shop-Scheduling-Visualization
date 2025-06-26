#include "Widget.h"
#include <iostream>

Widget::Widget()
{
	m_dragged = false;
	m_hSliderDragged = false;
	m_vSliderDragged = false;
	m_HsliderShape.setFillColor(WidgetUi::sliderColor);
	m_VsliderShape.setFillColor(WidgetUi::sliderColor);
	m_sliderBackgroundShape.setFillColor(WidgetUi::sliderBackgroundColor);
}

void Widget::draw(sf::RenderWindow& renderTarget)
{
	sf::RectangleShape rs;
	rs.setFillColor(sf::Color::White);
	rs.setSize({ m_widgetRect.width, m_widgetRect.height });
	rs.setPosition({ m_widgetRect.left ,m_widgetRect.top });
	renderTarget.draw(rs);

	sf::Vector2f windowSize = (sf::Vector2f)renderTarget.getSize();
	sf::FloatRect viewPortRect = {
		m_widgetRect.left / windowSize.x,
		m_widgetRect.top / windowSize.y,
		m_widgetRect.width / windowSize.x,
		m_widgetRect.height / windowSize.y
	}; 
	sf::View currentView = renderTarget.getView();
	m_View.setViewport(viewPortRect);
	renderTarget.setView(m_View);
	draw_internal(renderTarget);
	renderTarget.setView(currentView);
	drawSliders(renderTarget);
	draw_internalNoView(renderTarget);
}

void Widget::update(sf::RenderWindow& renderTarget, int mouseWheelDelta)
{
	sf::Vector2i mp = sf::Mouse::getPosition(renderTarget);
	sf::Vector2f mpc = renderTarget.mapPixelToCoords(mp, m_View);

	// camera moving
	if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && m_widgetRect.contains((sf::Vector2f)mp)) {
		if (!m_dragged) {
			m_dragged = true;
			m_dragStartPos = mpc;
		}
		else {
			sf::Vector2f dpos = m_dragStartPos - mpc;
			m_View.setCenter(m_View.getCenter() + dpos);
		}
	}
	else {
		m_dragged = false;
	}

	// V and H sliders
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		if (m_hSliderDragged) {
			float delta = m_sliderDragStartPos - mp.x;
			float hsliderLen = m_widgetRect.width - 2.0 * WidgetUi::sliderLenOffset;
			float move = (delta / hsliderLen) * m_sceneRectBorder.width;
			m_View.setCenter(m_centerPosSliderDragStart - sf::Vector2f(move, 0));
		}
		else if (m_vSliderDragged) {
			float delta = m_sliderDragStartPos - mp.y;
			float vsliderLen = m_widgetRect.height - 2.0 * WidgetUi::sliderLenOffset;
			float move = (delta / vsliderLen) * m_sceneRectBorder.height;
			m_View.setCenter(m_centerPosSliderDragStart - sf::Vector2f(0, move));
		}
		else if (m_HsliderShape.getGlobalBounds().contains((sf::Vector2f)mp)) {
			if (!m_hSliderDragged && !m_vSliderDragged) {
				m_hSliderDragged = true;
				m_centerPosSliderDragStart = m_View.getCenter();
				m_sliderDragStartPos = mp.x;
			}
		}
		else if (m_VsliderShape.getGlobalBounds().contains((sf::Vector2f)mp)) {
			if (!m_hSliderDragged && !m_vSliderDragged) {
				m_vSliderDragged = true;
				m_centerPosSliderDragStart = m_View.getCenter();
				m_sliderDragStartPos = mp.y;
			}
		}
	}
	else {
		m_vSliderDragged = m_hSliderDragged = false;
	}

	// Zoom
	if (m_widgetRect.contains((sf::Vector2f)mp)) {
		if (mouseWheelDelta > 0) {
			scale(1.15);
		}
		else if (mouseWheelDelta < 0) {
			scale(0.85);
		}
	}
	borderConstraintCheck();

	update_internal(renderTarget);
}

void Widget::setBorderRect(sf::FloatRect borderRect)
{
	m_sceneRectBorder = borderRect;
}

void Widget::setRect(sf::FloatRect widgetRect)
{
	m_widgetRect = widgetRect;
	m_View.setSize(m_widgetRect.width, m_widgetRect.height);
	//m_View.setCenter(m_widgetRect.width / 2.0, m_widgetRect.height / 2.0); 
}

float Widget::getScale()
{
	return m_View.getSize().x / m_widgetRect.width;
}

void Widget::scale(float factor)
{
	sf::Vector2f size = m_View.getSize();
	m_View.setSize(size.x * factor, size.y * factor);
}

void Widget::centerToTopLeft()
{
	m_View.setCenter({ 0,0 });
}

void Widget::draw_internal(sf::RenderWindow& renderTarget)
{
	return;
}

void Widget::draw_internalNoView(sf::RenderWindow& renderTarget)
{
	return;
}

void Widget::update_internal(sf::RenderWindow& renderTarget)
{
	return;
}

void Widget::borderConstraintCheck()
{
	sf::Vector2f size = m_View.getSize();
	if (size.x > m_sceneRectBorder.width) {
		float sf = m_sceneRectBorder.width / size.x;
		size.x = m_sceneRectBorder.width;
		size.y = size.y * sf;
	}
	if (size.y > m_sceneRectBorder.height) {
		float sf = m_sceneRectBorder.height / size.y;
		size.y = m_sceneRectBorder.height;
		size.x = size.x * sf;
	}
	m_View.setSize(size);

	sf::Vector2f cPos = m_View.getCenter();
	float left = cPos.x - size.x / 2.0;
	float top = cPos.y - size.y / 2.0;
	float right = cPos.x + size.x / 2.0;
	float bottom = cPos.y + size.y / 2.0;
	if (left < m_sceneRectBorder.left) {
		cPos += {m_sceneRectBorder.left - left, 0};
	}
	else if (right > m_sceneRectBorder.left + m_sceneRectBorder.width) {
		cPos -= {right - m_sceneRectBorder.left - m_sceneRectBorder.width, 0};
	}
	if (top < m_sceneRectBorder.top) {
		cPos += {0, m_sceneRectBorder.top - top};
	}
	else if (bottom > m_sceneRectBorder.top + m_sceneRectBorder.height) {
		cPos -= {0, bottom - m_sceneRectBorder.top - m_sceneRectBorder.height};
	}
	m_View.setCenter(cPos);
}

void Widget::drawSliders(sf::RenderWindow& renderTarget)
{
	sf::Vector2f viewSize = m_View.getSize();
	sf::Vector2f viewPos = m_View.getCenter();

	float xOccupacyLevel = viewSize.x / m_sceneRectBorder.width; // 0-1
	if (xOccupacyLevel < 0.999) {
		// horizontal slider background
		float hsliderLen = m_widgetRect.width - 2.0 * WidgetUi::sliderLenOffset;
		m_sliderBackgroundShape.setSize({ hsliderLen, WidgetUi::sliderThicness });
		m_sliderBackgroundShape.setPosition(
			m_widgetRect.left + WidgetUi::sliderLenOffset,
			m_widgetRect.top + m_widgetRect.height - WidgetUi::sliderBorderSpaceOffset - WidgetUi::sliderThicness
		);
		renderTarget.draw(m_sliderBackgroundShape);
		// horizontal slider 
		float xLeftPosLevel = (viewPos.x - viewSize.x / 2.0) / m_sceneRectBorder.width;
		m_HsliderShape.setSize({ xOccupacyLevel * hsliderLen, WidgetUi::sliderThicness });
		m_HsliderShape.setPosition(
			m_widgetRect.left + WidgetUi::sliderLenOffset + xLeftPosLevel * hsliderLen,
			m_widgetRect.top + m_widgetRect.height - WidgetUi::sliderBorderSpaceOffset - WidgetUi::sliderThicness);
		renderTarget.draw(m_HsliderShape);
	}

	float yOccupacyLevel = viewSize.y / m_sceneRectBorder.height; // 0-1
	if (yOccupacyLevel < 0.999) {
		// vertical slider background
		float vsliderLen = m_widgetRect.height - 2.0 * WidgetUi::sliderLenOffset;
		m_sliderBackgroundShape.setSize({ WidgetUi::sliderThicness, vsliderLen });
		m_sliderBackgroundShape.setPosition(
			m_widgetRect.left + m_widgetRect.width - WidgetUi::sliderBorderSpaceOffset - WidgetUi::sliderThicness,
			m_widgetRect.top + WidgetUi::sliderLenOffset
		);
		renderTarget.draw(m_sliderBackgroundShape);
		// vertical slider 
		float yTopPosLevel = (viewPos.y - viewSize.y / 2.0) / m_sceneRectBorder.height;
		m_VsliderShape.setSize({ WidgetUi::sliderThicness, yOccupacyLevel * vsliderLen });
		m_VsliderShape.setPosition(
			m_widgetRect.left + m_widgetRect.width - WidgetUi::sliderBorderSpaceOffset - WidgetUi::sliderThicness,
			m_widgetRect.top + WidgetUi::sliderLenOffset + yTopPosLevel * vsliderLen
		);
		renderTarget.draw(m_VsliderShape);
	}
}
