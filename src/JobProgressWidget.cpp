#include "JobProgressWidget.h"

JobProgressWidget::JobProgressWidget()
{
	m_font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
	m_textShape.setFont(m_font);
	m_textShape.setCharacterSize(JobProgrssUi::textCharSize);
	m_textShape.setFillColor(sf::Color::Black);
	m_jobContainer = nullptr;

	m_jobBack.setFillColor(sf::Color::Transparent);
	m_jobBack.setOutlineColor(sf::Color(150, 150, 150));
	m_jobBack.setOutlineThickness(JobProgrssUi::jobBackBorderThicness);
	m_jobBack.setSize(sf::Vector2f(
		500 - 2.0 * JobProgrssUi::jobBackBorderThicness, 
		JobProgrssUi::layerHeight - 2.0 * JobProgrssUi::jobBackBorderThicness));
	m_jobTitleBack.setSize(sf::Vector2f(500, JobProgrssUi::titleBackHeight));
	m_maxLayerCount = 0;
}

void JobProgressWidget::setJobContainer(fjss::JobContainer* jobContainer)
{
	m_jobContainer = jobContainer;
	m_jobUis.clear();
	m_maxLayerCount = 0;

	const std::map<fjss::JobID, fjss::Job>& jobs = jobContainer->getJobs();
	for (const auto& it : jobs) {
		JobUi jobUi = JobUi(it.second);
		m_maxLayerCount = std::max((int)m_maxLayerCount, (int)jobUi.m_operationsPerLayer.size());
		m_jobUis.push_back(jobUi);
	}

	float jobWidth = JobProgrssUi::jobGraphOffset.x + m_maxLayerCount * JobProgrssUi::layerSpaces;
	m_jobBack.setSize(sf::Vector2f(
		jobWidth - 2.0 * JobProgrssUi::jobBackBorderThicness,
		JobProgrssUi::layerHeight - 2.0 * JobProgrssUi::jobBackBorderThicness));
	m_jobTitleBack.setSize(sf::Vector2f(jobWidth, JobProgrssUi::titleBackHeight));

	m_sceneRectBorder.width = jobWidth;
	setBorderRect(m_sceneRectBorder);
}

void JobProgressWidget::setColorMap(std::unordered_map<int, sf::Color> colorJobIdMap)
{
	m_colorJobIdMap = colorJobIdMap;
}

void JobProgressWidget::draw_internal(sf::RenderWindow& renderTarget)
{
	sf::Vector2f viewSize = m_View.getSize();
	int jobCount = m_jobUis.size();
	
	for (int jobID = 0; jobID < jobCount; ++jobID) {
		m_jobBack.setPosition(
			JobProgrssUi::jobBackBorderThicness, 
			JobProgrssUi::layerHeight * jobID + JobProgrssUi::jobBackBorderThicness);
		renderTarget.draw(m_jobBack);
		m_jobTitleBack.setPosition(0, JobProgrssUi::layerHeight * jobID + JobProgrssUi::jobBackBorderThicness);
		m_jobTitleBack.setFillColor(m_colorJobIdMap[jobID]);
		renderTarget.draw(m_jobTitleBack);

		m_textShape.setCharacterSize(JobProgrssUi::textCharSize);
		m_textShape.setFillColor(sf::Color::Black);
		m_textShape.setString("Job " + std::to_string(jobID));
		m_textShape.setPosition(m_jobTitleBack.getPosition() + JobProgrssUi::titleTextOffset);
		renderTarget.draw(m_textShape);
		m_textShape.setFillColor(sf::Color::White);
		m_textShape.move(sf::Vector2f(-1, -1));
		renderTarget.draw(m_textShape);

		sf::Vector2f graphPos = JobProgrssUi::jobGraphOffset;
		graphPos.y += jobID * JobProgrssUi::layerHeight;
		drawJob(m_jobUis[jobID], graphPos, renderTarget);
	}
}

sf::Vector2f JobProgressWidget::operationIdToGraphPos(const JobUi& jobUi, fjss::OperationID operationID)
{
	std::pair<int, int> layerID_order = jobUi.m_operationLayer[operationID];
	int operationsInLayer = jobUi.m_operationsPerLayer[layerID_order.first];
	float layerSpacing = JobProgrssUi::layerHeight / (float)(operationsInLayer + 1);
	sf::Vector2f pos = {
				layerID_order.first * JobProgrssUi::layerSpaces,
				layerSpacing * (layerID_order.second + 1)
	};
	return pos;
}

void JobProgressWidget::update_internal(sf::RenderWindow& renderTarget)
{

}

void JobProgressWidget::drawLine(sf::Vector2f start, sf::Vector2f end, sf::RenderWindow& window, sf::Color color)
{
	static sf::RectangleShape line;
	line.setPosition(start);
	sf::Vector2f delta = start - end;
	float len = std::sqrt(std::pow(delta.x, 2) + std::pow(delta.y, 2));
	line.setSize(sf::Vector2f(len, 3));
	const double m_pi = acos(-1);
	float angle = (atan(delta.y / delta.x) * 180.0) / m_pi;
	if (end.x <= start.x) angle -= 180;
	line.setRotation(angle);
	line.setFillColor(color);
	window.draw(line);
}

void JobProgressWidget::drawJob(const JobUi& jobUi, sf::Vector2f pos, sf::RenderWindow& renderTarget)
{
	sf::Vector2i mp = sf::Mouse::getPosition(renderTarget);
	sf::Vector2f mpc = renderTarget.mapPixelToCoords(mp, m_View);

	sf::CircleShape csh(20);
	csh.setOrigin(20, 20);
	//csh.setFillColor(sf::Color::White);
	csh.setOutlineThickness(2.0);

	const std::map<fjss::OperationID, fjss::Operation>& operationVec = jobUi.m_job.getOperaions();

	// draw lines
	for (const auto& it : operationVec) {
		sf::Vector2f operationPos = operationIdToGraphPos(jobUi, it.first);
		//std::vector<fjss::OperationID> predecessors = it.second.getPredecessors();
		std::vector<fjss::OperationID> successors = jobUi.m_job.getSuccessors(it.first);
		for (const auto& successorID : successors) {
			sf::Vector2f predecessorPos = operationIdToGraphPos(jobUi, successorID);
			if (it.second.isDone())
				drawLine(operationPos + pos, predecessorPos + pos, renderTarget, sf::Color(205, 205, 205));
			else 
				drawLine(operationPos + pos, predecessorPos + pos, renderTarget, sf::Color::Black);
		}
	}
	// draw points
	for (const auto& it : operationVec) {
		sf::Vector2f operationPos = operationIdToGraphPos(jobUi, it.first);
		csh.setPosition(operationPos + pos);
		csh.setOutlineColor(sf::Color::Black);
		if (it.second.isDone()) {
			csh.setFillColor(sf::Color(205, 205, 205));
			csh.setOutlineColor(sf::Color(205, 205, 205));
		}
		else if (it.second.isAvailible()) {
			csh.setFillColor(sf::Color(165, 191, 250));
		}
		else {
			csh.setFillColor(sf::Color::White);
		}
		renderTarget.draw(csh);
		m_textShape.setCharacterSize(JobProgrssUi::textCharSize);
		m_textShape.setFillColor(sf::Color::Black);
		m_textShape.setString("O" + std::to_string(it.first));
		m_textShape.setPosition(operationPos + JobProgrssUi::operationTextOffset + pos);
		renderTarget.draw(m_textShape);
	}
}



JobUi::JobUi(const fjss::Job& job)
	: m_job(job)
{
	init();
}

void JobUi::init()
{
	const std::map<fjss::OperationID, fjss::Operation>& operationVec = m_job.getOperaions();
	m_operationLayer.resize(operationVec.size());
	fjss::Job job = m_job;
	job.restartJob();
	int layer = 0;
	while (job.isDone() == false) {
		std::vector<fjss::OperationID> avbOps = job.getAvailibleOperations();
		m_operationsPerLayer.push_back(avbOps.size());
		int layerOrder = 0;
		for (const auto& it : avbOps) {
			m_operationLayer[it] = { layer, layerOrder };
			job.dumpOperation(it, 0);
			++layerOrder;
		}
		++layer;
	}
}
