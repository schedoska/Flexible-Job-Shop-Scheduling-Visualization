#include "ConstructionAlgorithms.h"

namespace ConstructionAlgorithm {
	ConstructionSolver::ConstructionSolver(JobContainer& _jobContainer, Schedule& _schedule, ConstrutionFunction _constructionFunc)
		:jobContainer(_jobContainer), schedule(_schedule), constructionFunc(_constructionFunc)
	{
		jobContainer.restartContainer();
	}

	JobContainer& ConstructionSolver::getJobContainer()
	{
		return jobContainer;
	}

	Schedule& ConstructionSolver::getSchedule()
	{
		return schedule;
	}

	bool ConstructionSolver::isDone()
	{
		return jobContainer.isDone();
	}

	void ConstructionSolver::scheduleStep(int stepCount)
	{
		for (int i = 0; i < stepCount; ++i) {
			if (jobContainer.isDone()) return;
			ScheduleDecision sd = constructionFunc(jobContainer, schedule);
			schedule.stackScheduleOperation(sd.stationID, sd.operationID, sd.jobID, jobContainer);
		}
	}

	void ConstructionSolver::scheduleAll()
	{
		while (!jobContainer.isDone()) {
			ScheduleDecision sd = constructionFunc(jobContainer, schedule);
			schedule.stackScheduleOperation(sd.stationID, sd.operationID, sd.jobID, jobContainer);
		}
	}

	int dispatch_operation_mode = 0;
	int dispatch_station_mode = 0;
	std::vector<double> params;

	// ------------------------------ Planning algorithms

	ScheduleDecision RNG_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}
		// pick random operation form availible
		std::pair<OperationID, JobID> opIDJobID = avbOps[rand() % avbOps.size()];
		const Job& job = jobContainer.getJob(opIDJobID.second);
		const Operation& operation = job.getOperation(opIDJobID.first);
		std::vector<OperationTimeStation> ots;
		for (const OperationTimeStation& it : operation.getOperationTimeStations()) {
			ots.push_back(it);
		}
		// pick random station to insert
		sd.stationID = ots[rand() % ots.size()].stationID;
		sd.operationID = opIDJobID.first;
		sd.jobID = opIDJobID.second;
		return sd;
	}

	ScheduleDecision EIT_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}
		// find which operation can be inserted fastest and has most successors
		int earliestInsertTime = std::numeric_limits<int>::max();
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int insertTime = schedule.fastestTimeForScheduleOperation(operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				if (insertTime < earliestInsertTime) {
					earliestInsertTime = insertTime;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision EET_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}
		// find which operation can be inserted fastest
		int earliestEndTime = std::numeric_limits<int>::max();
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				if (endTime < earliestEndTime) {
					earliestEndTime = endTime;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision MWKR_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// find job with longest remaining time
		float longestRemainingTime = -1.0;
		for (const auto& jobIT : jobContainer.getJobs()) {
			float remT = jobIT.second.remainingAverageProcessTime();
			if (remT > longestRemainingTime) {
				longestRemainingTime = remT;
				sd.jobID = jobIT.first;
			}
		}
		const Job& job = jobContainer.getJob(sd.jobID);
		// find which operation can be ended fastest
		int earliestEndTime = std::numeric_limits<int>::max();
		for (auto& operationID : job.getAvailibleOperations()) {
			const fjss::Operation& operation = job.getOperation(operationID);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationID,
					sd.jobID,
					jobContainer);
				if (endTime < earliestEndTime) {
					earliestEndTime = endTime;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision LWKR_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// find job with longest remaining time
		float leastRemainingTime = std::numeric_limits<float>::max();
		for (const auto& jobIT : jobContainer.getJobs()) {
			float remT = jobIT.second.remainingAverageProcessTime();
			if (remT == 0) continue;
			if (remT < leastRemainingTime) {
				leastRemainingTime = remT;
				sd.jobID = jobIT.first;
			}
		}
		const Job& job = jobContainer.getJob(sd.jobID);
		// find which operation can be ended fastest
		int earliestEndTime = std::numeric_limits<int>::max();
		for (auto& operationID : job.getAvailibleOperations()) {
			const fjss::Operation& operation = job.getOperation(operationID);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationID,
					sd.jobID,
					jobContainer);
				if (endTime < earliestEndTime) {
					earliestEndTime = endTime;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision SPT_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}
		// find which operation is shortest
		int shortestDuration = std::numeric_limits<int>::max();
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				if (shortestDuration > operationStationTime.time) {
					shortestDuration = operationStationTime.time;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision LPT_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}
		// find which operation is shortest
		int longestDuration = -1.0;
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				if (longestDuration < operationStationTime.time) {
					longestDuration = operationStationTime.time;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision MS_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}
		// find which operation is shortest
		int earliestEndTime = std::numeric_limits<int>::max();
		int mostSuccessors = -1;
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);
			
			int successorCount = job.getSuccessors(operationJobID.first).size();
			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				if (successorCount > mostSuccessors ||
					(successorCount == mostSuccessors && endTime < earliestEndTime)) {
					earliestEndTime = endTime;
					mostSuccessors = successorCount;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision MS_LAM_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}
		// find which operation is shortest
		int earliestEndTime = std::numeric_limits<int>::max();
		int mostSuccAlt = -10000; 
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			int successorCount = job.getSuccessors(operationJobID.first).size();
			int alternativeCount = operation.getOperationTimeStations().size();
			int sa = successorCount - alternativeCount;
			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				if (sa > mostSuccAlt ||
					(mostSuccAlt == sa && endTime < earliestEndTime)) {
					earliestEndTime = endTime;
					mostSuccAlt = sa;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision MWKR_MS_LAM_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// find job with longest remaining time
		float longestRemainingTime = -1.0;
		for (const auto& jobIT : jobContainer.getJobs()) {
			float remT = jobIT.second.remainingAverageProcessTime();
			if (remT > longestRemainingTime) {
				longestRemainingTime = remT;
				sd.jobID = jobIT.first;
			}
		}
		const Job& job = jobContainer.getJob(sd.jobID);

		// find which operation has best sa value and eet
		int earliestEndTime = std::numeric_limits<int>::max();
		int mostSuccAlt = -10000;
		for (auto& operationID : job.getAvailibleOperations()) {
			const fjss::Operation& operation = job.getOperation(operationID);

			int successorCount = job.getSuccessors(operationID).size();
			int alternativeCount = operation.getOperationTimeStations().size();
			int sa = successorCount - alternativeCount;
			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationID,
					job.jobID,
					jobContainer);
				if (sa > mostSuccAlt ||
					(mostSuccAlt == sa && endTime < earliestEndTime)) {
					earliestEndTime = endTime;
					mostSuccAlt = sa;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision MMW_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}
		// find which operation is shortest
		int earliestEndTime = std::numeric_limits<int>::max();
		int minimalMachineWaste = std::numeric_limits<int>::max();
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				int machineEndTIme = schedule.getStationAvabilityTime(operationStationTime.stationID);
				int machineWaste = endTime - operationStationTime.time - machineEndTIme;

				if (machineWaste < minimalMachineWaste ||
					(machineWaste == minimalMachineWaste && endTime < earliestEndTime)) {
					earliestEndTime = endTime;
					minimalMachineWaste = machineWaste;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision HF1_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}
		// find operation + station that minimize v function
		int currentMakeSpan = schedule.makeSpan();
		float minV = std::numeric_limits<float>::max();
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			float thisJobRemainingTime = job.remainingAverageProcessTime();
			int alternativeStationCount = operation.getOperationTimeStations().size();
			int successorCount = job.getSuccessors(operation.operationID).size();

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				int startTime = endTime - operationStationTime.time;
				int stationWaste = endTime - schedule.getStationAvabilityTime(operationStationTime.stationID)
					- operationStationTime.time;
				int envelope = endTime - currentMakeSpan;

				float V = startTime + envelope + alternativeStationCount * 30.0 - thisJobRemainingTime - successorCount * 30.0;
				if (V < minV) {
					minV = V;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision HF2_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}
		// find operation + station that minimize v function
		int currentMakeSpan = schedule.makeSpan();
		float minV = std::numeric_limits<float>::max();
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			float thisJobRemainingTime = job.remainingAverageProcessTime();
			int alternativeStationCount = operation.getOperationTimeStations().size();
			int successorCount = job.getSuccessors(operation.operationID).size();

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				int startTime = endTime - operationStationTime.time;
				int stationWaste = endTime - schedule.getStationAvabilityTime(operationStationTime.stationID)
					- operationStationTime.time;
				int envelope = endTime - currentMakeSpan;

				// osttatnie paramertey z 27.05
				//float V = 6.27 * startTime + 2.08 * envelope + alternativeStationCount * 64.3 - thisJobRemainingTime - successorCount * 43.5;

				// ostatnia sktruktura z 27.05 godz 11:31
				//float V = startTime + params[0] * envelope + params[1] * alternativeStationCount +
				//	params[2] * thisJobRemainingTime + params[3] * successorCount;

				// zmiana 03.06.25
				//float V = envelope + params[0] * alternativeStationCount +
				//	params[1] * thisJobRemainingTime + params[2] * successorCount;
				float V = envelope + params[0] * alternativeStationCount +
					params[1] * job.remainingNumOfOperations() + params[2] * successorCount;

				if (V < minV) {
					minV = V;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision LPT_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}
		// find operation with longest avg processing time
		float longestProcessingTime = -1.0;
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);
			float pt = operation.averageProcessTime();
			if (pt > longestProcessingTime) {
				longestProcessingTime = pt;
				sd.jobID = job.jobID;
				sd.operationID = operation.operationID;
			}
		}

		// find which operation is shortest end
		int earliestEndTime = std::numeric_limits<int>::max();
		const fjss::Job& job = jobContainer.getJob(sd.jobID);
		const fjss::Operation& operation = job.getOperation(sd.operationID);
		for (auto& operationStationTime : operation.getOperationTimeStations()) {
			int endTime = schedule.fastestTimeForScheduleOperation(
				operationStationTime.stationID,
				sd.operationID,
				sd.jobID,
				jobContainer);

			if (endTime < earliestEndTime) {
				earliestEndTime = endTime;
				sd.stationID = operationStationTime.stationID;
			}
		}
		return sd;
	}

	ScheduleDecision MD_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// build station demand map
		std::map<StationID, int> stationDemandMap;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::map<OperationID, Operation>& jobOperationsMap = job.getOperaions();
			for (const auto& operationIT : jobOperationsMap) {
				if (!operationIT.second.isDone()) {
					for (auto& ots : operationIT.second.getOperationTimeStations()) {
						stationDemandMap[ots.stationID]++;
					}
				}
			}
		}
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which operation is shortest
		int earliestEndTime = std::numeric_limits<int>::max();
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				endTime += stationDemandMap[operationStationTime.stationID];

				if (endTime < earliestEndTime) {
					earliestEndTime = endTime;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision EIT_OT_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which operation is shortest
		int earliestInsertTime = std::numeric_limits<int>::max();
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int insertTime = schedule.fastestTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);

				// Osttanie parametry z 27.05
				//insertTime = (operationStationTime.time - operation.getShortestProcessTime()) + insertTime;

				insertTime = insertTime + params[0] * (operationStationTime.time - operation.getShortestProcessTime());

				if (insertTime < earliestInsertTime) {
					earliestInsertTime = insertTime;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision TBOP_HF_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		float tm = 0;
		// build station demand map
		std::map<StationID, int> stationDemandMap;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::map<OperationID, Operation>& jobOperationsMap = job.getOperaions();
			for (const auto& operationIT : jobOperationsMap) {
				if (!operationIT.second.isDone()) {
					for (auto& ots : operationIT.second.getOperationTimeStations()) {
						stationDemandMap[ots.stationID]++;
					}
					++tm;
				}
			}
		}
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which operation is shortest
		float earliestEndTime = std::numeric_limits<int>::max();
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);
			float timeBlockedByOperation = job.avgTimeBlockedByOperation(operation.operationID);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {

				// Ostatnie parametry z 27.05
				/*
				float endTime = schedule.fastestTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer) * 10.0;
				//int stationWaste = endTime - schedule.getStationAvabilityTime(operationStationTime.stationID);
				endTime -= 0.2*timeBlockedByOperation;
				endTime = 1.0*(operationStationTime.time - operation.getShortestProcessTime())
					+ endTime + 50*stationDemandMap[operationStationTime.stationID];
				*/

				float endTime = schedule.fastestTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				
				// z dnia 03.06
				//endTime = endTime + params[0] * timeBlockedByOperation +
					//params[1] * (operationStationTime.time - operation.getShortestProcessTime()) +
					//params[2] * stationDemandMap[operationStationTime.stationID];

				endTime = endTime + params[0] * timeBlockedByOperation +
					params[1] * (operationStationTime.time - operation.getShortestProcessTime()) +
					params[2] * ((float)stationDemandMap[operationStationTime.stationID] / tm) * operationStationTime.time;

				if (endTime < earliestEndTime) {
					earliestEndTime = endTime;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision TBOP_Planner(const JobContainer& jobContainer, const Schedule& schedule) {
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which operation is shortest
		float earliestEndTime = std::numeric_limits<int>::max();
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);
			float timeBlockedByOperation = job.avgTimeBlockedByOperation(operation.operationID);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				float endTime = schedule.fastestTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				endTime = endTime - timeBlockedByOperation;

				if (endTime < earliestEndTime) {
					earliestEndTime = endTime;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}


	ScheduleDecision TBOP_LA_EET_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which operation is shortest
		float earliestEndTime = std::numeric_limits<int>::max();
		float minLama = std::numeric_limits<int>::max();

		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);
			float timeBlockedByOperation = job.avgTimeBlockedByOperation(operation.operationID);
			int lama = -(int)operation.getOperationTimeStations().size();

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				float endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				endTime -= timeBlockedByOperation;

				if (lama < minLama || (lama == minLama && endTime < earliestEndTime)) {
					earliestEndTime = endTime;
					minLama = lama;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision NC_CP_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// find job with longest critical pAth
		float longestCP = -2.0;
		for (const auto& jobIT : jobContainer.getJobs()) {
			float cp = jobIT.second.nc_criticalPath();
			if (jobIT.second.isDone()) continue;
			if (cp > longestCP) {
				longestCP = cp;
				sd.jobID = jobIT.first;
			}
		}
		const Job& job = jobContainer.getJob(sd.jobID);

		// find which operation has best sa value and eet
		int earliestEndTime = std::numeric_limits<int>::max();
		for (auto& operationID : job.getAvailibleOperations()) {
			const fjss::Operation& operation = job.getOperation(operationID);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationID,
					job.jobID,
					jobContainer);
				if (endTime < earliestEndTime) {
					earliestEndTime = endTime;
					//sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision CP_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which operation is shortest
		float earliestEndTime = std::numeric_limits<int>::max();

		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);
			float cpl = job.criticalPath(operation.operationID);
			int altMach = operation.getOperationTimeStations().size();

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				float endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				// OSTTANIE PARAMETRY z 27.05
				//endTime = endTime - cpl;

				endTime = endTime - params[0] * cpl;

				if (endTime < earliestEndTime) {
					earliestEndTime = endTime;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision CP_HF_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		float tm = 0;
		// build station demand map
		std::map<StationID, int> stationDemandMap;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::map<OperationID, Operation>& jobOperationsMap = job.getOperaions();
			for (const auto& operationIT : jobOperationsMap) {
				if (!operationIT.second.isDone()) {
					for (auto& ots : operationIT.second.getOperationTimeStations()) {
						stationDemandMap[ots.stationID]++;
					}
					++tm;
				}
			}
		}
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which operation is shortest
		float makeSpan = schedule.makeSpan();
		float earliestEndTime = std::numeric_limits<int>::max();
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);
			float cpl = job.criticalPath(operation.operationID);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				float endTime = schedule.fastestTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);

				// manual
				//endTime = 10 * endTime - 0.2 * cpl
				//	+ 1 * (operationStationTime.time - operation.getShortestProcessTime())
				//	+ 50 * stationDemandMap[operationStationTime.stationID];  // 1169 insert time nie end
				// auto
				// 5.35223 - 0.430275    3.2423   59.5938
				
				// ostatnie parametry poni¿ej z 27.05
				//endTime = 5.35223 * endTime - 0.430275 * cpl
				//	+ 3.2423 * (operationStationTime.time - operation.getShortestProcessTime())
				//	+ 59.5938 * stationDemandMap[operationStationTime.stationID];  // 1141 insert time nie end

				// wersja do 03.06
				//endTime = endTime + params[0] * cpl + params[1] * (operationStationTime.time - operation.getShortestProcessTime()) +
				//	params[2] * stationDemandMap[operationStationTime.stationID];

				endTime = endTime + params[0] * cpl + params[1] * (operationStationTime.time - operation.getShortestProcessTime()) +
					params[2] * ((float)stationDemandMap[operationStationTime.stationID] / tm) * operationStationTime.time;

				if (endTime < earliestEndTime) {
					earliestEndTime = endTime;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision ELFT_Planner(const JobContainer& jobContainer, const Schedule& schedule)
	{
		ScheduleDecision sd;
		// all currently availible operation from all jobs
		std::vector<std::pair<OperationID, JobID>> avbOps;
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which operation is shortest
		float earliestEndTime = std::numeric_limits<int>::max();

		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);
			int LF = job.calculateLF(operation.operationID, schedule);
			int ES = job.calculateES(operation.operationID, schedule);
			int slack = LF - ES - operation.getShortestProcessTime();
			int altStations = operation.getOperationTimeStations().size();

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				float endTime = schedule.fastestEndTimeForScheduleOperation(
					operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				endTime = endTime + slack;

				if (endTime < earliestEndTime) {
					earliestEndTime = endTime;
					sd.jobID = job.jobID;
					sd.operationID = operation.operationID;
					sd.stationID = operationStationTime.stationID;
				}
			}
		}
		return sd;
	}

	ScheduleDecision LVL_Planner(const JobContainer & jobContainer, const Schedule & schedule)
	{
		ScheduleDecision sd;
		// select operation
		float v = std::numeric_limits<int>::max();
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) {
				const fjss::Operation& operation = job.getOperation(operationID);
				float slackTime = -job.criticalPath(operationID);
				if (slackTime < v) {
					v = slackTime;
					sd.jobID = job.jobID;
					sd.operationID = operationID;
				}
			}
		}

		// find which station is shortest
		float earliestEndTime = std::numeric_limits<int>::max();
		const fjss::Job& job = jobContainer.getJob(sd.jobID);
		const fjss::Operation& operation = job.getOperation(sd.operationID);
		for (const OperationTimeStation& ots : operation.getOperationTimeStations()) {
			float endTime = schedule.fastestEndTimeForScheduleOperation(
				ots.stationID,
				sd.operationID,
				sd.jobID,
				jobContainer);

			if (endTime < earliestEndTime) {
				earliestEndTime = endTime;
				sd.stationID = ots.stationID;
			}
		}
		return sd;
	}

	ScheduleDecision Dispatch_Planner(const JobContainer& jobContainer, const Schedule& schedule) {
		ScheduleDecision sd;
		// select operation
		float minV = std::numeric_limits<int>::max();
		for (const auto& jobIT : jobContainer.getJobs()) {
			const fjss::Job& job = jobIT.second;
			const std::vector<OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) {
				const fjss::Operation& operation = job.getOperation(operationID);

				float v = dispatch_operation(job, operation, jobContainer, schedule);
				if (v < minV) {
					minV = v;
					sd.jobID = job.jobID;
					sd.operationID = operationID;
				}
			}
		}

		// find which station is shortest
		float minM = std::numeric_limits<int>::max();
		const fjss::Job& job = jobContainer.getJob(sd.jobID);
		const fjss::Operation& operation = job.getOperation(sd.operationID);
		for (const OperationTimeStation& ots : operation.getOperationTimeStations()) {

			float m = dispatch_station(ots, job, operation, jobContainer, schedule);
			if (m < minM) {
				minM = m;
				sd.stationID = ots.stationID;
			}
		}
		return sd;
	}

	float dispatch_operation(const Job& job, const Operation& operation, const JobContainer& jobContainer, const Schedule& schedule) {
		switch (dispatch_operation_mode) {
		case 0:	
			// Job most work remaining
			return -job.remainingAverageProcessTime();
		case 1: 
			// longest critical path after operation
			return -job.criticalPath(operation.operationID);
		case 2: 
			// most avg time blocked by operation
			return -job.avgTimeBlockedByOperation(operation.operationID);
		case 3: 
			// least alternative station
			return operation.getOperationTimeStations().size();		
		case 4: 
			// longest critical path after operation + alternative machines
			return -job.criticalPath(operation.operationID) / operation.getOperationTimeStations().size();
		case 5:
			// most operation number remaining
			return -job.remainingNumOfOperations();
		case 6:
			// least operation number remaining
			return job.remainingNumOfOperations();
		case 7:
			// Job least work remaining
			return job.remainingAverageProcessTime();
		case 8: 
			// most upstream successors
			return -job.getSuccessorsUpstream(operation.operationID);
		case 9:
			// most upstream successors
			return -(float)job.getSuccessorsUpstream(operation.operationID) + 0.1 * (float)operation.getOperationTimeStations().size();
		}
	}

	float dispatch_station(const OperationTimeStation& ots, const Job& job, const Operation& operation, const JobContainer& jobContainer, const Schedule& schedule) {
		switch (dispatch_station_mode) {
		case 0:
			// EET
			return schedule.fastestEndTimeForScheduleOperation(ots.stationID, operation.operationID, job.jobID, jobContainer);
		case 1:
			// build station demand map
		{
			std::map<StationID, int> stationDemandMap;
			float allOps = 0;
			float tm = 0;
			for (const auto& jobIT : jobContainer.getJobs()) {
				const fjss::Job& job = jobIT.second;
				const std::map<OperationID, Operation>& jobOperationsMap = job.getOperaions();
				for (const auto& operationIT : jobOperationsMap) {
					if (!operationIT.second.isDone()) {
						for (auto& ots : operationIT.second.getOperationTimeStations()) {
							++stationDemandMap[ots.stationID];
							++allOps;
						}
						++tm;
					}
				}
			}
			//std::cout << stationDemandMap[ots.stationID] << " / " << allOps << "\n";
			int envelope = schedule.makeSpan() - schedule.fastestEndTimeForScheduleOperation(ots.stationID, operation.operationID, job.jobID, jobContainer);
			//return -envelope + ((float)stationDemandMap[ots.stationID] / allOps) * operation.averageProcessTime();
			//return -envelope + ((float)stationDemandMap[ots.stationID] / tm) * operation.averageProcessTime();
			return -envelope + ((float)stationDemandMap[ots.stationID] / tm) * ots.time;
		}
		case 2:
			// shortest processing time
			return ots.time;
		}
		}
	}



