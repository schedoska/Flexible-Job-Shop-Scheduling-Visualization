#pragma once

#include "Widget.h"
#include "FJSS.hpp"
#include <unordered_map>

class ScheduleWidget : public Widget
{
public:
	ScheduleWidget(); 
	void setContext(fjss::Schedule* schedule, fjss::JobContainer* jobContainer);
	void setColorMap(std::unordered_map<int, sf::Color> colorJobIdMap);

private:
	void draw_internal(sf::RenderWindow& renderTarget) override;
	void update_internal(sf::RenderWindow& renderTarget) override;
	void draw_internalNoView(sf::RenderWindow& renderTarget) override;

	void drawGrid(sf::RenderWindow& renderTarget);
	sf::RectangleShape m_verticalLineShape;
	sf::RectangleShape m_horizontalLineShape;
	sf::RectangleShape m_timePtrShape;
	sf::Text m_textShape;
	sf::Font m_font;
	sf::CircleShape m_triangleShape;
	sf::RectangleShape m_makeSpanLineShape;

	void drawSchedule(sf::RenderWindow& renderTarget);
	fjss::Schedule* m_schedule;
	fjss::JobContainer* m_jobContainer;
	std::unordered_map<int, sf::Color> m_colorJobIdMap;

	float m_currentLongestOperation;
	fjss::ScheduledOperation m_followedOpJobID; // operationID, JobID
	bool m_jobFollowMode;

	float m_machineHeight;
	float m_pixelPerTime;
	bool m_keyButtonPressed;

	sf::RectangleShape m_PopUpInfoShape;
};

namespace ScheduleUi {
	const sf::Vector2f gridPos = { 40,30 };
	const float machineHeight = 70;
	const float machineSpaces = 10;
	const sf::Color gridColor = sf::Color::Black;
	const float gridThicness = 2;
	const float pixelsPerTime = 1.0;

	const int timePtrSpaces = 40;
	const float timePtrHeight = 5;
	const sf::Vector2f textPtrOffset = { -5, 10 };
	const float textCharSize = 13;
	const float textCharSizeOperation = 13;
	const sf::Color textOperationColor = sf::Color::White;
	const sf::Vector2f textOperationOffset = sf::Vector2f(5, 10);

	const float triangleRadius = 6;
	const sf::Vector2f textMachinePtrOffset = { -30, 0 };

	const sf::Vector2f popUpSize = sf::Vector2f(120, 69);
	const sf::Vector2f popUpOffset = sf::Vector2f(-popUpSize.x / 2.0, 30);
	const sf::Color popUpColor = sf::Color(102, 102, 102);
	const sf::Color underPopUpColor = sf::Color(70, 70, 70);
	const sf::Vector2f popUpTextOffset = sf::Vector2f(5, 5);

	const sf::Color makeSpanColor = sf::Color(140,140,140);
	const sf::Vector2f makeSpanTextOffset = sf::Vector2f(10, 0);
}

