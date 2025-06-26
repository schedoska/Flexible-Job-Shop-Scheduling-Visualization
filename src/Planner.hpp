//#include "Utils.hpp"
#include <limits>
#include "FJSS.hpp"

// ---------------------- 0 planning algoritm ----------------------------
// best machine + operation but jobs iteratively

fjss::StationID findBestAvailibleStation(const fjss::Operation& operation, const fjss::Job& job, fjss::JobContainer& jobContainer, const fjss::Schedule& schedule);

fjss::Schedule plan0(fjss::JobContainer& jobContainer, unsigned stationCount) {
	fjss::Schedule schedule(stationCount);

	for (auto& jobIT : jobContainer.getJobs()) {
		fjss::Job& job = jobIT.second;
		while (!job.isDone()) {
			const std::vector<fjss::OperationID>& avbOpsIDs = job.getAvailibleOperations();
			fjss::OperationID opID = avbOpsIDs.front();
			fjss::StationID bestStationID = findBestAvailibleStation(job.getOperation(opID), job, jobContainer, schedule);
			schedule.stackScheduleOperation(bestStationID, opID, job.jobID, jobContainer);
		}
	}
	return schedule;
}

fjss::StationID findBestAvailibleStation(const fjss::Operation& operation, const fjss::Job& job, fjss::JobContainer& jobContainer, const fjss::Schedule& schedule) {
	const std::vector<fjss::OperationTimeStation>& avbStations = operation.getOperationTimeStations();
	int quickestInsertTime = std::numeric_limits<int>::max();
	fjss::StationID bestStationID = 0;
	for (const fjss::OperationTimeStation& station : avbStations) {
		int t = schedule.fastestTimeForScheduleOperation(station.stationID, operation.operationID, job.jobID, jobContainer);
		if (t < quickestInsertTime) {
			quickestInsertTime = t;
			bestStationID = station.stationID;
		}
	}
	return bestStationID;
}

// ---------------------- 1 planning algoritm ----------------------------
// random dispatching = baseline

fjss::Schedule planRandom(fjss::JobContainer& jobContainer, unsigned stationCount) {
	fjss::Schedule schedule(stationCount);

	for (auto& jobIT : jobContainer.getJobs()) {
		fjss::Job& job = jobIT.second;
		while (!job.isDone()) {
			const std::vector<fjss::OperationID>& avbOpsIDs = job.getAvailibleOperations();
			fjss::OperationID opID = avbOpsIDs.front();
			const fjss::Operation& operation = job.getOperation(opID);
			int randomStation = rand() % operation.getOperationTimeStations().size();
			fjss::StationID bestStationID = operation.getOperationTimeStations()[randomStation].stationID;
			schedule.stackScheduleOperation(bestStationID, opID, job.jobID, jobContainer);
		}
	}
	return schedule;
}

// ---------------------- 2 planning algoritm ----------------------------
// Best machine + operation gloabl

fjss::Schedule plan2(fjss::JobContainer& jobContainer, unsigned stationCount) {
	fjss::Schedule schedule(stationCount);
	
	while (jobContainer.isDone() == false) {
		// all currently availible operation from all jobs
		std::vector<std::pair<fjss::OperationID, fjss::JobID>> avbOps;
		for (auto& jobIT : jobContainer.getJobs()) {
			fjss::Job& job = jobIT.second;
			const std::vector<fjss::OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which operation can be inserted fastest
		int quickestInsertTime = std::numeric_limits<int>::max();
		fjss::StationID bestStationID = 0;
		fjss::OperationID bestOperationID = 0;
		fjss::JobID bestJobID = 0;
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int time = schedule.fastestTimeForScheduleOperation(operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				if (time < quickestInsertTime) {
					quickestInsertTime = time;
					bestStationID = operationStationTime.stationID;
					bestOperationID = operationJobID.first;
					bestJobID = operationJobID.second;
				}
			}
		}
		schedule.stackScheduleOperation(bestStationID, bestOperationID, bestJobID, jobContainer);
	}
	return schedule;
}

// ---------------------- 3 planning algoritm ----------------------------
// Minimize stations Load
fjss::Schedule plan3(fjss::JobContainer& jobContainer, unsigned stationCount) {
	fjss::Schedule schedule(stationCount);

	while (jobContainer.isDone() == false) {
		// all currently availible operation from all jobs
		std::vector<std::pair<fjss::OperationID, fjss::JobID>> avbOps;
		for (auto& jobIT : jobContainer.getJobs()) {
			fjss::Job& job = jobIT.second;
			const std::vector<fjss::OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which machine is avalible quickest and insert it there
		int lowestStationLoad = std::numeric_limits<int>::max();
		fjss::StationID bestStationID = 0;
		fjss::OperationID bestOperationID = 0;
		fjss::JobID bestJobID = 0;

		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int stationLoad = schedule.getStationAvabilityTime(operationStationTime.stationID);
				if (stationLoad < lowestStationLoad) {
					lowestStationLoad = stationLoad;
					bestStationID = operationStationTime.stationID;
					bestOperationID = operationJobID.first;
					bestJobID = operationJobID.second;
				}
			}
		}
		schedule.stackScheduleOperation(bestStationID, bestOperationID, bestJobID, jobContainer);
	}
	return schedule;
}


// ---------------------- 4 planning algoritm ----------------------------
// Longest Remaining Processing Time

float jobRemainingProcessingTime(const fjss::Job& job) {
	const std::map<fjss::OperationID, fjss::Operation>& operations = job.getOperaions();
	float remainingTime = 0;
	for (const auto& operation : operations) {
		if (operation.second.isDone()) 
			continue;
		float avgStationTime = 0;
		for (const auto& operationStationTime : operation.second.getOperationTimeStations()) {
			avgStationTime += operationStationTime.time;
		}
		remainingTime += (avgStationTime / (float)operation.second.getOperationTimeStations().size());
	}
	return remainingTime;
}

fjss::Schedule plan4(fjss::JobContainer& jobContainer, unsigned stationCount) {
	fjss::Schedule schedule(stationCount);

	while (jobContainer.isDone() == false) {
		// 1. find longest remaining time job
		float longestRemainingTime = 0;
		fjss::JobID bestJobID = 0;
		for (const auto& job : jobContainer.getJobs()) {
			float remainingTime = jobRemainingProcessingTime(job.second);
			if (remainingTime > longestRemainingTime) {
				longestRemainingTime = remainingTime;
				bestJobID = job.first;
			}
		}

		// 2. find best job + station combinantion inside job
		const std::vector<fjss::OperationID>& avbOps = jobContainer.getJob(bestJobID).getAvailibleOperations();
		int quickestEndTime = std::numeric_limits<int>::max();
		fjss::StationID bestStationID = 0;
		fjss::OperationID bestOperationID = 0;
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(bestJobID);
			const fjss::Operation& operation = job.getOperation(operationJobID);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(operationStationTime.stationID,
					operationJobID,
					bestJobID,
					jobContainer);
				if (endTime < quickestEndTime) {
					quickestEndTime = endTime;
					bestStationID = operationStationTime.stationID;
					bestOperationID = operationJobID;
				}
			}
		}
		schedule.stackScheduleOperation(bestStationID, bestOperationID, bestJobID, jobContainer);
	}
	return schedule;
}


// ---------------------- 5 planning algoritm ----------------------------
// 1. Find job that can be inserted fastest and has the most successors

fjss::Schedule plan5(fjss::JobContainer& jobContainer, unsigned stationCount) {
	fjss::Schedule schedule(stationCount);

	while (jobContainer.isDone() == false) {
		// all currently availible operation from all jobs
		std::vector<std::pair<fjss::OperationID, fjss::JobID>> avbOps;
		for (auto& jobIT : jobContainer.getJobs()) {
			fjss::Job& job = jobIT.second;
			const std::vector<fjss::OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which operation can be inserted fastest and has most successors
		int quickestInsertTime = std::numeric_limits<int>::max();
		int highestSuccessorCount = -1;
		fjss::StationID bestStationID = 0;
		fjss::OperationID bestOperationID = 0;
		fjss::JobID bestJobID = 0;
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int insetTime = schedule.fastestEndTimeForScheduleOperation(operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				int successorCount = job.getSuccessors(operationJobID.first).size();

				if (successorCount > highestSuccessorCount ||
					((successorCount == highestSuccessorCount) && insetTime < quickestInsertTime)) {
					quickestInsertTime = insetTime;
					bestStationID = operationStationTime.stationID;
					bestOperationID = operationJobID.first;
					bestJobID = operationJobID.second;
					highestSuccessorCount = successorCount;
				}
			}
		}
		schedule.stackScheduleOperation(bestStationID, bestOperationID, bestJobID, jobContainer);
	}
	return schedule;
}


// ---------------------- 6 planning algoritm ----------------------------
// 1. Find job that will take on average the most and has the most successors

fjss::Schedule plan6(fjss::JobContainer& jobContainer, unsigned stationCount) {
	fjss::Schedule schedule(stationCount);

	while (jobContainer.isDone() == false) {
		// all currently availible operation from all jobs
		std::vector<std::pair<fjss::OperationID, fjss::JobID>> avbOps;
		for (auto& jobIT : jobContainer.getJobs()) {
			fjss::Job& job = jobIT.second;
			const std::vector<fjss::OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// find which operation has longest avg processing time and has highest successor count
		float longsetProcessTime = 0;
		int highestSuccessorCount = -1;
		fjss::OperationID bestOperationID = 0;
		fjss::JobID bestJobID = 0;
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);
			float processTime = operation.averageProcessTime();
			int successorCount = job.getSuccessors(operationJobID.first).size();

			if (successorCount > highestSuccessorCount ||
				((successorCount == highestSuccessorCount) && processTime > longsetProcessTime)) {
				longsetProcessTime = processTime;
				highestSuccessorCount = successorCount;
				bestOperationID = operationJobID.first;
				bestJobID = operationJobID.second;
			}
		}

		// find on which station it can be finished the fastest
		int quickestEndTime = std::numeric_limits<int>::max();
		fjss::StationID bestStationID = 0;
		const fjss::Job& job = jobContainer.getJob(bestJobID);
		const fjss::Operation& operation = job.getOperation(bestOperationID);
		for (auto& operationStationTime : operation.getOperationTimeStations()) {
			int endTime = schedule.fastestEndTimeForScheduleOperation(operationStationTime.stationID,
				bestOperationID,
				bestJobID,
				jobContainer);
			if (endTime < quickestEndTime) {
				quickestEndTime = endTime;
				bestStationID = operationStationTime.stationID;
			}
		}
		schedule.stackScheduleOperation(bestStationID, bestOperationID, bestJobID, jobContainer);
	}
	return schedule;
}



// ---------------------- 7 planning algoritm ----------------------------
// heuristic function

fjss::Schedule plan7(fjss::JobContainer& jobContainer, unsigned stationCount) {
	fjss::Schedule schedule(stationCount);

	while (jobContainer.isDone() == false) {
		// all currently availible operation from all jobs
		std::vector<std::pair<fjss::OperationID, fjss::JobID>> avbOps;
		for (auto& jobIT : jobContainer.getJobs()) {
			fjss::Job& job = jobIT.second;
			const std::vector<fjss::OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// evaluate best operation selection
		float bestV = std::numeric_limits<float>::max();
		fjss::OperationID bestOperationID = 0;
		fjss::JobID bestJobID = 0;
		fjss::StationID bestStationID = 0;
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);
			
			int quickestEndTime = 100000;
			fjss::StationID bestStationIDloc = 0;
			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				if (endTime < quickestEndTime) {
					quickestEndTime = endTime;
					bestStationIDloc = operationStationTime.stationID;
				}
			}
			int stationHoleSize = quickestEndTime - schedule.getStationAvabilityTime(bestStationIDloc)
				- operation.getProcessTimeOnStationID(bestStationIDloc);
			int successorCount = job.getSuccessors(operationJobID.first).size();
			float thisJobRemainingTime = jobRemainingProcessingTime(job);
			float operationAvgTime = operation.averageProcessTime();

			// the smaller the better
			float v = stationHoleSize - thisJobRemainingTime + quickestEndTime - operationAvgTime;
			if (v < bestV) {
				bestV = v;
				bestOperationID = operation.operationID;
				bestJobID = job.jobID;
				bestStationID = bestStationIDloc;
			}
		}
		schedule.stackScheduleOperation(bestStationID, bestOperationID, bestJobID, jobContainer);
	}
	return schedule;
}


// ---------------------- 8 planning algoritm ----------------------------
// heuristic function with envelope
fjss::Schedule plan8(fjss::JobContainer& jobContainer, unsigned stationCount) {
	fjss::Schedule schedule(stationCount);

	while (jobContainer.isDone() == false) {
		// all currently availible operation from all jobs
		std::vector<std::pair<fjss::OperationID, fjss::JobID>> avbOps;
		for (auto& jobIT : jobContainer.getJobs()) {
			fjss::Job& job = jobIT.second;
			const std::vector<fjss::OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		int currentMakeSpan = schedule.makeSpan();

		// evaluate best operation selection
		float bestV = std::numeric_limits<float>::max();
		fjss::OperationID bestOperationID = 0;
		fjss::JobID bestJobID = 0;
		fjss::StationID bestStationID = 0;
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			float thisJobRemainingTime = jobRemainingProcessingTime(job);
			int alternativeStationCount = operation.getOperationTimeStations().size();

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTime = schedule.fastestEndTimeForScheduleOperation(operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);
				int startTime = endTime - operationStationTime.time;

				int stationWaste = endTime - schedule.getStationAvabilityTime(operationStationTime.stationID)
					- operationStationTime.time;

				int envelope = endTime - currentMakeSpan;
				int successorCount = job.getSuccessors(operationJobID.first).size();
				//float V = startTime + envelope + alternativeStationCount * 30.0 - thisJobRemainingTime - successorCount * 30.0;
				//float V = 9.22*startTime + 1.98*envelope + alternativeStationCount * 50.8 - thisJobRemainingTime - successorCount * 47.2;
				//float V = 16.98 * startTime + 5.12 * envelope + alternativeStationCount * 160.8 - thisJobRemainingTime - successorCount * 104.581;
				float V = 6.27*startTime + 2.08*envelope + alternativeStationCount * 64.3 - thisJobRemainingTime - successorCount * 43.5;
				if (V < bestV) {
					bestV = V;
					bestOperationID = operationJobID.first;
					bestJobID = operationJobID.second;
					bestStationID = operationStationTime.stationID;
				}
			}
		}
		schedule.stackScheduleOperation(bestStationID, bestOperationID, bestJobID, jobContainer);
	}
	return schedule;
}

// ---------------------- 9 planning algoritm ----------------------------
// Just simple Earliest insert time
fjss::Schedule plan9(fjss::JobContainer& jobContainer, unsigned stationCount) {
	fjss::Schedule schedule(stationCount);

	while (jobContainer.isDone() == false) {
		// all currently availible operation from all jobs
		std::vector<std::pair<fjss::OperationID, fjss::JobID>> avbOps;
		for (auto& jobIT : jobContainer.getJobs()) {
			fjss::Job& job = jobIT.second;
			const std::vector<fjss::OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// evaluate best operation selection
		float longestRemainingJob = 10e7;
		float earliestEndTime = 10e6;

		fjss::OperationID bestOperationID = 0;
		fjss::JobID bestJobID = 0;
		fjss::StationID bestStationID = 0;
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			float thisJobRemainingTime = jobRemainingProcessingTime(job);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTIme = schedule.fastestEndTimeForScheduleOperation(operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);

				if (thisJobRemainingTime < longestRemainingJob ||
					endTIme < earliestEndTime && thisJobRemainingTime <= longestRemainingJob) {
					earliestEndTime = endTIme;
					longestRemainingJob = thisJobRemainingTime;
					bestOperationID = operationJobID.first;
					bestJobID = operationJobID.second;
					bestStationID = operationStationTime.stationID;
				}
			}
		}
		schedule.stackScheduleOperation(bestStationID, bestOperationID, bestJobID, jobContainer);
	}
	return schedule;
}


// ---------------------- 10 planning algoritm ----------------------------
// Calculating stationdemand
fjss::Schedule plan10(fjss::JobContainer& jobContainer, unsigned stationCount) {
	fjss::Schedule schedule(stationCount);

	// get stationDemandMap
	std::map<fjss::StationID, int> stationDemandMap;
	for (auto& jobIT : jobContainer.getJobs()) {
		fjss::Job& job = jobIT.second;
		const auto& JobOperations = job.getOperaions();
		for (auto& operation : JobOperations) {
			const std::vector<fjss::OperationTimeStation>& ots = operation.second.getOperationTimeStations();
			for (auto& timeStation : ots) {
				stationDemandMap[timeStation.stationID] += 1;
			}
		}
	}

	while (jobContainer.isDone() == false) {
		// all currently availible operation from all jobs
		std::vector<std::pair<fjss::OperationID, fjss::JobID>> avbOps;
		for (auto& jobIT : jobContainer.getJobs()) {
			fjss::Job& job = jobIT.second;
			const std::vector<fjss::OperationID>& jobAvbOps = job.getAvailibleOperations();
			for (const auto& operationID : jobAvbOps) avbOps.push_back({ operationID, job.jobID });
		}

		// evaluate best operation selection
		int leastDemandStation = 10e7;
		int earliestEndTime = 10e7;

		fjss::OperationID bestOperationID = 0;
		fjss::JobID bestJobID = 0;
		fjss::StationID bestStationID = 0;
		int bestDuration = 0;
		for (auto& operationJobID : avbOps) {
			const fjss::Job& job = jobContainer.getJob(operationJobID.second);
			const fjss::Operation& operation = job.getOperation(operationJobID.first);

			for (auto& operationStationTime : operation.getOperationTimeStations()) {
				int endTIme = schedule.fastestEndTimeForScheduleOperation(operationStationTime.stationID,
					operationJobID.first,
					operationJobID.second,
					jobContainer);

				if (stationDemandMap[operationStationTime.stationID] < leastDemandStation ||
					endTIme < earliestEndTime && stationDemandMap[operationStationTime.stationID] <= leastDemandStation) {
					earliestEndTime = endTIme;
					leastDemandStation = stationDemandMap[operationStationTime.stationID];
					bestOperationID = operationJobID.first;
					bestJobID = operationJobID.second;
					bestStationID = operationStationTime.stationID;
					bestDuration = operationStationTime.time;
				}
			}
		}
		stationDemandMap[bestStationID] -= 1;
		schedule.stackScheduleOperation(bestStationID, bestOperationID, bestJobID, jobContainer);
	}
	return schedule;
}
