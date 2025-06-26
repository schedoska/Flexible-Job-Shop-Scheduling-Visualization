#pragma once
#include <SFML/Graphics.hpp>
#include <thread>
#include "ScheduleWidget.h"
#include "JobProgressWidget.h"
#include "ConstructionAlgorithms.h"

class Window
{
public:
	Window();
	void start();
	ScheduleWidget m_scheduleWidget;
	JobProgressWidget m_jobProgressWidget;
	ConstructionAlgorithm::ConstructionSolver* m_constructionSolver;
	void setConstructionSolver(ConstructionAlgorithm::ConstructionSolver* constructionSolver);

private:
	void updateResizeWidgets();
	sf::RenderWindow m_window;
};

