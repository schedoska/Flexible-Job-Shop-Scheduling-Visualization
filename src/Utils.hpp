#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>

using OperationID = size_t;
using JobID = size_t;
using StationID = size_t;

template <typename T>
void printContainer(const T& ctr) {
	for (const auto& elem : ctr) std::cout << elem << " ";
	std::cout << std::endl;
}

struct OperationTimeStation {
	StationID stationID;
	int time;
};

struct Operation {
	Operation() {
		lastPredecessorEndTime = 0;
	};

	Operation(
		const OperationID id,
		const std::vector<OperationID>& successors,
		const std::set<OperationID>& predecessors,
		const std::vector<OperationTimeStation>& operationTimeStation) {
		this->id = id,
			this->successors = successors;
		this->predecessors = predecessors;
		this->operationTimeStations = operationTimeStation;
		lastPredecessorEndTime = 0;
	}

	void precedessorDoneUpdate(OperationID predecessorId, int endTime) {
		predecessors.erase(predecessorId);
		lastPredecessorEndTime = std::max(lastPredecessorEndTime, endTime);
	}

	bool isAvailible() const {
		return !predecessors.size();
	}

	int getProcessTimeForStationID(StationID stationID) const {
		for (const OperationTimeStation& ots : operationTimeStations)
			if (ots.stationID == stationID) return ots.time;
		return -1;
	}

	void addSuccessor(OperationID successor) {
		successors.push_back(successor);
	}

	void addPredecessor(OperationID predecessor) {
		predecessors.insert(predecessor);
	}

	void addOperationTimeStation(const OperationTimeStation& operationTimeStation) {
		this->operationTimeStations.push_back(operationTimeStation);
	}

	void setID(OperationID id) {
		this->id = id;
	}

	OperationID id;
	std::vector<OperationID> successors;
	std::set<OperationID> predecessors;	// Cant start before empty
	std::vector<OperationTimeStation> operationTimeStations; // station id / time
	int lastPredecessorEndTime;
};

struct Job {
	Job() = default;

	Job(JobID id) {
		this->id = id;
	}

	void addOperation(const Operation& operation) {
		operations[operation.id] = operation;
	}

	void addOperations(const std::vector<Operation>& operationVec) {
		for (const Operation& op : operationVec) operations[op.id] = op;
	}

	void setID(JobID id) {
		this->id = id;
	}

	JobID id;
	std::map<OperationID, Operation> operations;
	std::vector<OperationID> availibleOperations;

	bool isFinished() const {
		return !availibleOperations.size();
	}

	const std::vector<OperationID>& getAvailibleOPerations() const {
		return availibleOperations;
	}

	const Operation& getOperation(OperationID id) {
		return operations[id];
	}

	bool dumpOperation(const OperationID operationId, int endTime) {
		auto idItr = std::find(availibleOperations.begin(), availibleOperations.end(), operationId);
		if (idItr == availibleOperations.end()) {
			return false;
		}
		availibleOperations.erase(idItr);
		// infrom all succesors and check their availibity
		for (OperationID& suc : operations[operationId].successors) {
			operations[suc].precedessorDoneUpdate(operationId, endTime);
			if (operations[suc].isAvailible())
				availibleOperations.push_back(suc);
		}
		return true;
	};
};

struct OperationProxy
{
	OperationProxy(OperationID operationID = 0, JobID jobID = 0, int startTime = 0, int duration = 0) {
		this->operationID = operationID;
		this->jobID = jobID;
		this->startTime = startTime;
		this->duration = duration;
	}

	JobID jobID;
	OperationID operationID;
	int startTime;
	int duration;

	int calculateEndTime() const {
		return startTime + duration;
	}
};

struct JobContainer {
	std::map<JobID, Job> jobs;

	void addJobs(const std::vector<Job>& jobs) {
		for (const Job& job : jobs) this->jobs[job.id] = job;
	}

	bool allJobFinished() {
		for (const auto& j : jobs) {
			if (!j.second.isFinished()) return false;
		}
		return true;
	}

	void addJob(const Job& job) {
		jobs[job.id] = job;
	}
};

struct Schedule {

	std::vector<std::vector<OperationProxy>> stations; // stationId, operation order

	Schedule(size_t numberOfStations) {
		stations.resize(numberOfStations);
	}

	void scheduleStackOperation(StationID stationID, const Operation& operation, Job& job) {
		if (stationID >= stations.size())
			throw std::runtime_error("stackOPeration: Station [" + std::to_string(stationID) + "] doesn't exist");

		int lastPrecedessorEndTime = operation.lastPredecessorEndTime;
		int stationAvblTime = stationTimeAvailibity(stationID);
		int duration = operation.getProcessTimeForStationID(stationID);
		if (duration == -1)
			throw std::runtime_error("stackOPeration: Operation [" + std::to_string(operation.id) +
				"] can't be processed on station [" + std::to_string(stationID) + "]");

		OperationProxy proxy(operation.id, job.id, std::max(lastPrecedessorEndTime, stationAvblTime), duration);
		stations[stationID].push_back(proxy);
		job.dumpOperation(operation.id, proxy.calculateEndTime());
	}

	int stationTimeAvailibity(StationID stationId) const {
		return stations[stationId].size() ? stations[stationId].back().calculateEndTime() : 0;
	}

	void print() const {
		size_t stationCount = stations.size();

		for (int statId = 0; statId < stationCount; ++statId) {
			size_t operationCount = stations[statId].size();
			std::cout << "M" << statId << ": ";

			for (int opOrder = 0; opOrder < operationCount; ++opOrder) {
				OperationProxy op = stations[statId][opOrder];
				std::cout << "o" << op.operationID << "(J" << op.jobID << ")";
				std::cout << "[" << op.startTime << "-" << op.calculateEndTime() << "] - ";
			}
			std::cout << std::endl;
		}
	}
};
