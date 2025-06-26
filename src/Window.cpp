#include "Window.h"
#include <iostream>

Window::Window()
{
	//m_window = sf::RenderWindow>(new sf::RenderWindow(sf::VideoMode(800, 600), "FJSS Visualization"));
	m_window.create(sf::VideoMode(1600, 800), "FJSS Visualization");
	m_window.setFramerateLimit(60);
	m_scheduleWidget.setRect(sf::FloatRect(10, 10, 1100, 600));
	m_scheduleWidget.setBorderRect(sf::FloatRect(0, 0, 2000, 600));

	m_jobProgressWidget.setRect(sf::FloatRect(1120, 10, 600, 780));
	m_jobProgressWidget.setBorderRect(sf::FloatRect(0, 0, 500, 3000));
	m_constructionSolver = nullptr;

	updateResizeWidgets();

	std::unordered_map<int, sf::Color> colorJobIdMap;
	for (int i = 0; i < 30; ++i) {
		colorJobIdMap[i] = sf::Color(rand() % 255, rand() % 255, rand() % 255);
	}
	m_scheduleWidget.setColorMap(colorJobIdMap);
	m_jobProgressWidget.setColorMap(colorJobIdMap);

	m_scheduleWidget.centerToTopLeft();
	m_jobProgressWidget.centerToTopLeft();
}

void Window::start()
{
	while (m_window.isOpen())
	{
		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		int mouseWheelTicks = 0;
		while (m_window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
				m_window.close();

			if (event.type == sf::Event::Resized)
			{
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				m_window.setView(sf::View(visibleArea));
				updateResizeWidgets();
			}
			if (event.type == sf::Event::MouseWheelMoved)
			{
				mouseWheelTicks = event.mouseWheel.delta;
			}
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Num1) {
					if (m_constructionSolver == nullptr) continue;
					if (m_constructionSolver->isDone()) continue;
					m_constructionSolver->scheduleStep();
				}
				if (event.key.code == sf::Keyboard::Num2) {
					if (m_constructionSolver == nullptr) continue;
					if (m_constructionSolver->isDone()) continue;
					m_constructionSolver->scheduleStep(10);
				}
				if (event.key.code == sf::Keyboard::Num3) {
					if (m_constructionSolver == nullptr) continue;
					m_constructionSolver->getSchedule().clear();
					m_constructionSolver->getJobContainer().restartContainer();
				}
			}
		}

		m_window.clear(sf::Color(200,200,200));
		m_scheduleWidget.draw(m_window);
		m_jobProgressWidget.draw(m_window);
		m_scheduleWidget.update(m_window, mouseWheelTicks);
		m_jobProgressWidget.update(m_window);
		m_window.display();
	}
}

void Window::setConstructionSolver(ConstructionAlgorithm::ConstructionSolver* constructionSolver)
{
	m_constructionSolver = constructionSolver;
	m_scheduleWidget.setContext(&m_constructionSolver->getSchedule(), &m_constructionSolver->getJobContainer());
	m_jobProgressWidget.setJobContainer(&m_constructionSolver->getJobContainer());
}

void Window::updateResizeWidgets()
{
	sf::Vector2f windowSize = (sf::Vector2f)m_window.getView().getSize();

	float jobProgressWidth = 500.0;
	float widgetSpacings = 10.0;
	m_scheduleWidget.setRect(
		sf::FloatRect(widgetSpacings, widgetSpacings, windowSize.x - jobProgressWidth - 3*widgetSpacings, 600));
	m_jobProgressWidget.setRect(
		sf::FloatRect(windowSize.x - jobProgressWidth - widgetSpacings, widgetSpacings, jobProgressWidth, windowSize.y - widgetSpacings));
	//m_jobProgressWidget.scale(10.0);
	//m_scheduleWidget.scale(10.0);
}
