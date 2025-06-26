#pragma once

#define MAX_SUCCESSORS 10
#define MAX_OPERATIONS_IN_JOB 15
#define MAX_OPERATION_TYPES 20
#define MAX_ELIGIBLE_MACHINES 3
#define MAX_JOBS_IN_PROBLEM 30

struct JobData
{
	int8_t initialPredecessorCount[MAX_OPERATIONS_IN_JOB];
	int8_t successorIDs[MAX_SUCCESSORS * MAX_OPERATIONS_IN_JOB];	// input operationID, get succesors
	int8_t typeIDs[MAX_OPERATIONS_IN_JOB]; // to convert operationID to typeID
};

struct OperationTypeData
{
	int8_t eligibleMachines[MAX_OPERATION_TYPES * MAX_ELIGIBLE_MACHINES * 2];	// input typeID, get eligible machines
	// [([0,34],[-1,-1],[-1,-1]), (...]
};

struct ProblemData 
{
	JobData jobData[MAX_JOBS_IN_PROBLEM];
};

// -------------------------------------------

struct OperationGPU {
	int8_t predecessorCount;
};

struct JobGPU {
	OperationGPU operations[MAX_OPERATIONS_IN_JOB];
};

// every Thread needs it's copy of this
struct ProblemGPU {
	JobGPU jobs[MAX_JOBS_IN_PROBLEM];
};