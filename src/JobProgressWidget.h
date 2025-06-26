#pragma once

#include "Widget.h"
#include "FJSS.hpp"
#include <unordered_map>
#include <cmath>

struct JobUi {
	JobUi(const fjss::Job& job);
	std::vector<std::pair<int, int>> m_operationLayer; // operation id to layerID and order in layer
	std::vector<int> m_operationsPerLayer; // operation id to layer count
	const fjss::Job& m_job;

private:
	void init();
};

class JobProgressWidget : public Widget
{
public:
	JobProgressWidget();
	void setJobContainer(fjss::JobContainer* jobContainer);
	void setColorMap(std::unordered_map<int, sf::Color> colorJobIdMap);

private:
	fjss::JobContainer* m_jobContainer;
	std::vector<JobUi> m_jobUis;
	sf::Text m_textShape;
	sf::Font m_font;
	std::unordered_map<int, sf::Color> m_colorJobIdMap;

	sf::RectangleShape m_jobBack;
	sf::RectangleShape m_jobTitleBack;
	int m_maxLayerCount;

	void draw_internal(sf::RenderWindow& renderTarget) override;
	void update_internal(sf::RenderWindow& renderTarget) override;

	void drawLine(sf::Vector2f start, sf::Vector2f end, sf::RenderWindow& window, sf::Color color);
	void drawJob(const JobUi& jobUi, sf::Vector2f pos, sf::RenderWindow& renderTarget);
	sf::Vector2f operationIdToGraphPos(const JobUi& jobUi, fjss::OperationID operationID);
};

namespace JobProgrssUi {
	const float textCharSize = 15;
	const sf::Vector2f operationTextOffset = sf::Vector2f(-10, -10);
	const float jobBackBorderThicness = 4;

	const float layerHeight = 200.0;
	const float layerSpaces = 80.0;
	const sf::Vector2f jobGraphOffset = sf::Vector2f(40, 10);
	const float titleBackHeight = 23.0;
	const sf::Vector2f titleTextOffset = sf::Vector2f(12, 2);
}






