#include "ScheduleWidget.h"
#include <iostream>

ScheduleWidget::ScheduleWidget()
{
	m_verticalLineShape.setFillColor(ScheduleUi::gridColor);
	m_verticalLineShape.setPosition(ScheduleUi::gridPos);
	m_horizontalLineShape.setFillColor(ScheduleUi::gridColor);
	m_timePtrShape.setFillColor(ScheduleUi::gridColor);
	m_timePtrShape.setSize({ ScheduleUi::gridThicness, ScheduleUi::timePtrHeight });
	m_makeSpanLineShape.setFillColor(ScheduleUi::makeSpanColor);
	
	m_font.loadFromFile("C:\\Users\\chedo\\OneDrive\\Pulpit\\PARTICLE_FILTER\\OpenSans.ttf");
	m_textShape.setFont(m_font);
	m_textShape.setCharacterSize(ScheduleUi::textCharSize);
	m_textShape.setFillColor(ScheduleUi::gridColor);

	m_triangleShape.setRadius(ScheduleUi::triangleRadius);
	m_triangleShape.setOrigin(ScheduleUi::triangleRadius, ScheduleUi::triangleRadius);
	m_triangleShape.setPointCount(3);
	m_triangleShape.setFillColor(ScheduleUi::gridColor);

	for (int i = 0; i < 20; ++i) {
		m_colorJobIdMap[i] = sf::Color(rand() % 255, rand() % 255, rand() % 255);
	}

	m_schedule = nullptr;
	m_currentLongestOperation = 1;
	m_jobFollowMode = false;

	m_machineHeight = ScheduleUi::machineHeight;
	m_keyButtonPressed = false;
	m_pixelPerTime = ScheduleUi::pixelsPerTime;
}

void ScheduleWidget::setContext(fjss::Schedule* schedule, fjss::JobContainer* jobContainer)
{
	m_schedule = schedule;
	m_jobContainer = jobContainer;
	m_sceneRectBorder.height = (m_schedule->stationCount()+1.5) * (m_machineHeight + ScheduleUi::machineSpaces);
}

void ScheduleWidget::setColorMap(std::unordered_map<int, sf::Color> colorJobIdMap)
{
	m_colorJobIdMap = colorJobIdMap;
}

void ScheduleWidget::draw_internal(sf::RenderWindow& renderTarget)
{
	drawSchedule(renderTarget);
	drawGrid(renderTarget);
}

void ScheduleWidget::update_internal(sf::RenderWindow& renderTarget)
{
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		if (!m_jobFollowMode && m_followedOpJobID.jobID != -1) {
			m_jobFollowMode = true;
		}
	}
	else {
		m_jobFollowMode = false;
		m_followedOpJobID.jobID = -1;
	}

	bool upBtn = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
	bool downBtn = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
	bool rightBtn = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
	bool leftBtns = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
	bool btnPressed = upBtn || downBtn || rightBtn || leftBtns;
	
	if (btnPressed && !m_keyButtonPressed) {
		m_keyButtonPressed = true;
		float newPixelPerTime = m_pixelPerTime;
		float newMachineHeight = m_machineHeight;

		if (upBtn) newMachineHeight -= 10;
		else if (downBtn) newMachineHeight += 10;
		else if (rightBtn) newPixelPerTime *= (double)(5.0 / 4.0);
		else if (leftBtns) newPixelPerTime *= (double)(4.0 / 5.0);

		if (newPixelPerTime > 0.2f && newPixelPerTime < 20.f) m_pixelPerTime = newPixelPerTime;
		if (newMachineHeight > 10.f && newMachineHeight < 100.f) m_machineHeight = newMachineHeight;
	}
	else if (!btnPressed) {
		m_keyButtonPressed = false;
	}
}

void ScheduleWidget::draw_internalNoView(sf::RenderWindow& renderTarget)
{
	sf::Vector2i mp = sf::Mouse::getPosition(renderTarget);

	if (m_jobFollowMode) {
		m_PopUpInfoShape.setFillColor(ScheduleUi::popUpColor);
		m_PopUpInfoShape.setSize(ScheduleUi::popUpSize);
		m_PopUpInfoShape.setPosition((sf::Vector2f)mp + ScheduleUi::popUpOffset);
		renderTarget.draw(m_PopUpInfoShape);

		const fjss::Job& job = m_jobContainer->getJob(m_followedOpJobID.jobID);
		const fjss::Operation& operation = job.getOperation(m_followedOpJobID.operationID);
		const std::vector<fjss::OperationID>& predecessors = operation.getPredecessors();

		m_textShape.setFillColor(sf::Color::White);
		m_textShape.setCharacterSize(ScheduleUi::textCharSize);
		std::string s = "O" + std::to_string(m_followedOpJobID.operationID) + " (J" + std::to_string(m_followedOpJobID.jobID) + ")";
		s += " | M" + std::to_string(m_followedOpJobID.stationID) + "\n";
		s += std::to_string(m_followedOpJobID.startTime) + " - " + std::to_string(m_followedOpJobID.endTime());
		s += " [" + std::to_string(m_followedOpJobID.duration) + "]\n";
		s += "prec: ";
		for (int i = 0; i < predecessors.size(); ++i) {
			s += "O" + std::to_string(predecessors[i]);
			if (i < predecessors.size() - 1) s += ", ";
		}
		m_textShape.setString(s);
		m_textShape.setPosition(m_PopUpInfoShape.getPosition() + ScheduleUi::popUpTextOffset);
		renderTarget.draw(m_textShape);
		
		const std::vector<fjss::OperationTimeStation>& alternativeStations = operation.getOperationTimeStations();
		m_PopUpInfoShape.setFillColor(ScheduleUi::underPopUpColor);
		m_PopUpInfoShape.setSize(sf::Vector2f(ScheduleUi::popUpSize.x, 5 + (ScheduleUi::popUpSize.y / 3.0) * alternativeStations.size()));
		m_PopUpInfoShape.setPosition(m_PopUpInfoShape.getPosition() + sf::Vector2f(0, ScheduleUi::popUpSize.y));
		renderTarget.draw(m_PopUpInfoShape);
		std::string ms;
		for (const fjss::OperationTimeStation& ots : alternativeStations) {
			ms += "M" + std::to_string(ots.stationID) + " | " + std::to_string(ots.time) + "\n";
		}
		m_textShape.setString(ms);
		m_textShape.setPosition(m_PopUpInfoShape.getPosition() + ScheduleUi::popUpTextOffset);
		renderTarget.draw(m_textShape);
	}
}

void ScheduleWidget::drawGrid(sf::RenderWindow& renderTarget)
{
	int nummach = m_schedule->stationCount();
	int currentMakeSapan = m_schedule->makeSpan();
	//int currentMaxTime = 2400;
	int currentMaxTime = std::max((((currentMakeSapan+100) / 200) + 1) * 200.0, 1000.0);
	float timePtrSpaces = 40.0 / m_pixelPerTime;

	// draw vertical line
	float vlineLength = nummach * (m_machineHeight + ScheduleUi::machineSpaces)
		+ 1.0 * ScheduleUi::machineSpaces;
	m_verticalLineShape.setSize({ ScheduleUi::gridThicness, vlineLength });
	renderTarget.draw(m_verticalLineShape);

	// draw makespan line
	m_makeSpanLineShape.setSize({ ScheduleUi::gridThicness, vlineLength });
	m_makeSpanLineShape.setPosition(ScheduleUi::gridPos.x + m_pixelPerTime * currentMakeSapan, ScheduleUi::gridPos.y);
	renderTarget.draw(m_makeSpanLineShape);
	m_textShape.setFillColor(sf::Color::Black);
	m_textShape.setPosition(m_makeSpanLineShape.getPosition() + ScheduleUi::makeSpanTextOffset);
	m_textShape.setString("Makespan:\n" + std::to_string(currentMakeSapan));
	renderTarget.draw(m_textShape);

	// draw horizontal line
	float hlineLength = currentMaxTime * m_pixelPerTime;
	m_horizontalLineShape.setSize({ hlineLength, ScheduleUi::gridThicness });
	m_horizontalLineShape.setPosition(ScheduleUi::gridPos.x, ScheduleUi::gridPos.y + vlineLength);
	renderTarget.draw(m_horizontalLineShape);
	m_sceneRectBorder.width = std::max(hlineLength + 4 * ScheduleUi::gridPos.x, m_widgetRect.width);


	// draw machine pointers
	m_textShape.setFillColor(ScheduleUi::gridColor);
	m_textShape.setCharacterSize(ScheduleUi::textCharSize);
	for (int i = 0; i < nummach; ++i) {
		float machineHeight = m_machineHeight;
		sf::Vector2f pos = ScheduleUi::gridPos + sf::Vector2f(0, ScheduleUi::machineSpaces + machineHeight / 2.0);
		pos += sf::Vector2f(0, i * (machineHeight + ScheduleUi::machineSpaces));
		m_textShape.setString("M" + std::to_string(i));
		m_textShape.setPosition(pos + ScheduleUi::textMachinePtrOffset);
		renderTarget.draw(m_textShape);
	}

	// draw time pointers and text stamps
	for (int timePoint = 0; timePoint < currentMaxTime; timePoint += timePtrSpaces) {
		sf::Vector2f pos = {
			ScheduleUi::gridPos.x + timePoint * m_pixelPerTime,
			ScheduleUi::gridPos.y + vlineLength - ScheduleUi::timePtrHeight
		};
		m_timePtrShape.setPosition(pos);
		renderTarget.draw(m_timePtrShape);
		m_textShape.setString(std::to_string(timePoint));
		m_textShape.setPosition(pos + ScheduleUi::textPtrOffset);
		renderTarget.draw(m_textShape);
	}

	// draw traingles on axes
	float tr = ScheduleUi::triangleRadius;
	m_triangleShape.setRotation(0);
	m_triangleShape.setPosition(ScheduleUi::gridPos);
	renderTarget.draw(m_triangleShape);
	m_triangleShape.setRotation(90);
	m_triangleShape.setPosition(ScheduleUi::gridPos + sf::Vector2f(hlineLength, vlineLength));
	renderTarget.draw(m_triangleShape);
}

void ScheduleWidget::drawSchedule(sf::RenderWindow& renderTarget)
{
	if (m_schedule == nullptr) return;

	sf::Vector2i mp = sf::Mouse::getPosition(renderTarget);
	sf::Vector2f mpc = renderTarget.mapPixelToCoords(mp, m_View);

	sf::RectangleShape operationShape;
	operationShape.setOutlineColor(sf::Color::Black);
	sf::Vector2f npos = ScheduleUi::gridPos + sf::Vector2f(0, ScheduleUi::machineSpaces);

	const float mspace = m_machineHeight + ScheduleUi::machineSpaces;

	const std::vector<std::vector<fjss::ScheduledOperation>>& schduledOperations = m_schedule->getSchedule();
	for (const auto& machineVec : schduledOperations) {
		for (const auto& op : machineVec) {
			sf::Vector2f pos = npos + sf::Vector2f(op.startTime * m_pixelPerTime, op.stationID * mspace);
			operationShape.setPosition(pos);
			operationShape.setSize({ (float)op.duration * m_pixelPerTime, m_machineHeight });
			sf::Color color = m_colorJobIdMap[op.jobID];

			const float shrinkSize = 2;
			if (operationShape.getGlobalBounds().contains(mpc)) {
				operationShape.setOutlineThickness(shrinkSize);
				operationShape.setSize(operationShape.getSize() - sf::Vector2f(shrinkSize * 2, shrinkSize * 2));
				operationShape.setPosition(operationShape.getPosition() + sf::Vector2f(shrinkSize, shrinkSize));
				m_followedOpJobID = op;
			}
			else {
				operationShape.setOutlineThickness(0);
			}

			if (!m_jobFollowMode || m_followedOpJobID.jobID == op.jobID) {
				operationShape.setFillColor(m_colorJobIdMap[op.jobID]);
			}
			else {
				color.a = 50;
				operationShape.setFillColor(color);
			}
			renderTarget.draw(operationShape);

			if (op.duration * m_pixelPerTime > 43) {
				m_textShape.setFillColor(sf::Color::Black);
				m_textShape.setCharacterSize(ScheduleUi::textCharSize);
				m_textShape.setString("O" + std::to_string(op.operationID) + " (J" + std::to_string(op.jobID) + ")");
				m_textShape.setPosition(pos + ScheduleUi::textOperationOffset);
				renderTarget.draw(m_textShape);
				m_textShape.setFillColor(ScheduleUi::textOperationColor);
				m_textShape.move(-1, -1);
				renderTarget.draw(m_textShape);
			}
		}
	}
}
