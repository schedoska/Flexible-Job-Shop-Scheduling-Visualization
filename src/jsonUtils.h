#pragma once

#include "FJSS.hpp"
#include "json.hpp"
#include <fstream>

fjss::Job parseJob(const nlohmann::json& jobJSON, const nlohmann::json& jobPrecJSON) {
	fjss::Job job;
	int operationCount = jobJSON.size();

	for (int opIt = 0; opIt < operationCount; ++opIt) {
		fjss::Operation operation;
		operation.operationID = opIt;
		nlohmann::json operationStationJSON = jobJSON.at(opIt);
		nlohmann::json operationPrecJSON = jobPrecJSON.at(opIt);

		for (auto& stationIt : operationStationJSON) {
			fjss::OperationTimeStation operationTimeStation;
			operationTimeStation.time = stationIt.at(0);
			operationTimeStation.stationID = stationIt.at(1);
			operation.addOperationTimeStation(operationTimeStation);
		}
		for (auto& PrecIt : operationPrecJSON) {
			operation.addPredecessor(PrecIt);
		}
		job.addOperation(operation);
	}
	return job;
}

fjss::JobContainer parseProblem(const nlohmann::json& problemJSON) {
	fjss::JobContainer jobContainer;
	auto jobJSONIt = problemJSON["Jobs"].begin(); 
	auto jobPrecJSONIt = problemJSON["Prec"].begin();
	jobContainer.setStationCount(problemJSON["numM"]);

	for (fjss::JobID jobID = 0; jobJSONIt != problemJSON["Jobs"].end(); ++jobJSONIt, ++jobPrecJSONIt, ++jobID) {
		fjss::Job job = parseJob(*jobJSONIt, *jobPrecJSONIt);
		job.jobID = jobID;
		jobContainer.addJob(job);
	}
	return jobContainer;
}

fjss::Job parseBrandiMarteJob(std::ifstream& fileStream) {
	fjss::Job job;
	int operationCount;
	fileStream >> operationCount;

	for (int opID = 0; opID < operationCount; ++opID) {
		fjss::Operation operation;
		operation.operationID = opID;
		int stationCount;
		fileStream >> stationCount;

		for (int i = 0; i < stationCount; ++i) {
			fjss::OperationTimeStation operationTimeStation;
			fileStream >> operationTimeStation.stationID >> operationTimeStation.time;
			operation.addOperationTimeStation(operationTimeStation);
		}
		if (opID > 0) {
			operation.addPredecessor(opID - 1);
		}
		job.addOperation(operation);
	}
	return job;
}

fjss::JobContainer parseBrandiMarteProblem(std::ifstream& fileStream) {
	fjss::JobContainer jobContainer;
	int jobCount, stationCount;
	fileStream >> jobCount >> stationCount;
	jobContainer.setStationCount(stationCount);
	for (fjss::JobID jobID = 0; jobID < jobCount; ++jobID) {
		fjss::Job job = parseBrandiMarteJob(fileStream);
		job.jobID = jobID;
		jobContainer.addJob(job);
	}
	return jobContainer;
}




// Brandimarte generator
void generateBrandimarteProblemSet(std::vector<fjss::JobContainer>& problemSet, const std::string& sourceTypeFileName, int setSize) {
	std::ifstream sourceFile(sourceTypeFileName);
	fjss::JobContainer jobContainer = parseBrandiMarteProblem(sourceFile);
	int n_org = jobContainer.getJobs().size();
	int l_bound = 0.8 * n_org;
	int u_bound = 1.2 * n_org;

	for (int i = 0; i < setSize; ++i) {
		fjss::JobContainer problemInstance;
		int jobCount = l_bound + (rand() % (u_bound - l_bound + 1));
		problemInstance.setStationCount(jobContainer.stationCount());
		
		for (fjss::JobID j = 0; j < jobCount; ++j) {
			int jobType = rand() % n_org;
			fjss::Job job = jobContainer.getJob(jobType);
			job.jobID = j;
			problemInstance.addJob(job);
		}
		problemSet.push_back(problemInstance);
	}
}
