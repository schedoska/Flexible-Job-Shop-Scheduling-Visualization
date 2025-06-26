#pragma once

#include <limits>
#include "FJSS.hpp"
#include <functional>

using namespace fjss;

namespace ConstructionAlgorithm {
	extern int dispatch_operation_mode;
	extern int dispatch_station_mode;
	extern std::vector<double> params;

	struct ScheduleDecision {
		StationID stationID;
		OperationID operationID;
		JobID jobID;
	};

	class ConstructionSolver {
	public:
		using ConstrutionFunction = std::function<ScheduleDecision(const JobContainer& jobContainer, const Schedule& schedule)>;

		ConstructionSolver(
			JobContainer& jobContainer, 
			Schedule& schedule, 
			ConstrutionFunction constructionFunc
		);
		JobContainer& getJobContainer();
		Schedule& getSchedule();
		bool isDone();
		void scheduleStep(int stepCount = 1);
		void scheduleAll();

	private:
		JobContainer& jobContainer;
		Schedule& schedule;
		ConstrutionFunction constructionFunc;
	};


	
	/* Earliest insert time algorithm */
	ScheduleDecision EIT_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1673.47
	/* Earliest end time algorithm */
	ScheduleDecision EET_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1666.51
	/* Least work remaining (sum of avg operation time) + Earliest end time algorithm */
	ScheduleDecision LWKR_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1873.41
	/* Shortest processing operation */
	ScheduleDecision SPT_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 2364.09
	/* Shortest processing operation */
	ScheduleDecision LPT_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 4245.4
	/* Find longest avg proc time operation and EET */
	ScheduleDecision LPT_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 2380.45




	// Dispatch heuristics
	/* Most successors + earliest end time */
	ScheduleDecision MS_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1505.28
	/* Most successors + Least Alternative machines + earliest end time */
	ScheduleDecision MS_LAM_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1382.94
	/* Longest remaining JOb + Most successors + Least Alternative machines + earliest end time */
	ScheduleDecision MWKR_MS_LAM_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1403.37
	/* Minimize machine waste + EET */
	ScheduleDecision MMW_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 3697.98

	


	// Flops/  experimentals:
	/* Slack = LF - ES then EET = Hybrid ELFT-Slack*/
	ScheduleDecision ELFT_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1565.91
	/* Non correct Critical Path */
	ScheduleDecision NC_CP_Planner(const JobContainer& jobContainer, const Schedule& schedule);
	/* Heuristic Function Manual-tuned parameters */
	ScheduleDecision HF1_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1215.54
	/* Determine the demand for each station among not done ops (num of ops per machine) +
	   Select one with lowest  */
	ScheduleDecision MD_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1633.85
	/* Least alternative + most successors + Time blocked by operation */
	ScheduleDecision TBOP_LA_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1630.81
	/* Random dispatch algorithm */
	ScheduleDecision RNG_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 2420.25


	/* 2 poziomowe algorytmy */
	// 1. Least count of alternative stations -> 2. EET  // 1657.6
	// 1. longest critical path -> 2. EET  // 1384.67
	// 1. longest time blocked by operation-> 2. EET  // 1476.98
	// 1. SPT operation-> 2. EET  // 2280.01
	// 1. minimal Slack (LF-ES-PT) -> 2. EET // 2181.91
	ScheduleDecision LVL_Planner(const JobContainer& jobContainer, const Schedule& schedule);

	float dispatch_operation(const Job& job, const Operation& operation, const JobContainer& jobContainer, const Schedule& schedule);
	float dispatch_station(const OperationTimeStation& ots, const Job& job, const Operation& operation, const JobContainer& jobContainer, const Schedule& schedule);
	ScheduleDecision Dispatch_Planner(const JobContainer& jobContainer, const Schedule& schedule);





	// Final greedy algorithms:
	/* Heuristic Function libcmaes-tuned parameters */
	ScheduleDecision HF2_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1185.96
	/* Time blocked by operation + heuristic function */
	ScheduleDecision TBOP_HF_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1176.38
	/* Critical Path + EET */
	ScheduleDecision CP_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1245.18
	/* Critical Path + heuristic function */
	ScheduleDecision CP_HF_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1169.91 for manual

	// Weaker greedy algorithms:
	/* Time blocked by operation + EET */
	ScheduleDecision TBOP_Planner(const JobContainer& jobContainer, const Schedule& schedule);
	/* EIT + deviation from optimal time for operation */
	ScheduleDecision EIT_OT_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1343.76

	// Weaker hybrid algos:
	/* Most work remaining (sum of avg operation time) + Earliest end time algorithm */
	ScheduleDecision MWKR_Planner(const JobContainer& jobContainer, const Schedule& schedule); // 1362.19
}

