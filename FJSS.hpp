#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>

template <typename T>
void printContainer(const T& ctr) {
    for (const auto& elem : ctr) std::cout << elem << " ";
    std::cout << std::endl;
}

namespace fjss {
    using OperationID = size_t;
    using OperationTypeID = size_t;
    using JobID = size_t;
    using JobTypeID = size_t;
    using StationID = size_t;

    struct OperationTimeStation {
        StationID stationID;
        int time;
    };

    /* Operation class ==================================== */
    class Operation {
        std::vector<OperationTimeStation> m_operationTimeStations;
        std::vector<OperationID> m_predecessors;

        std::set<OperationID> m_predecessorsToDo;
        int m_lastPrecedessorTime;
        bool m_done;

    public:
        OperationID operationID;
        OperationTypeID operationTypeID;

        Operation();
        void addOperationTimeStation(const OperationTimeStation& operationTimeStation);
        void addPredecessor(OperationID operationID);
        void restartOperation();
        bool isAvailible() const;
        void precedessordoneUpdate(OperationID predecessorID, int endTime);
        int getLastPrecedessorTime() const;
        int getProcessTimeOnStationID(StationID stationID) const;
        void markDone();
        bool isDone() const;
        float averageProcessTime() const;
        int getShortestProcessTime() const;
        int getLongestProcessTime() const;

        const std::vector<OperationTimeStation>& getOperationTimeStations() const { return m_operationTimeStations; }
        const std::vector<OperationID>& getPredecessors() const { return m_predecessors; }
    };

    /* Job class ==================================== */
    class Schedule;

    struct Job {
        std::map<OperationID, Operation> m_operations;
        std::vector<OperationID> m_availibleOperations;
        std::map<OperationID, std::vector<OperationID>> m_successors;

    public:
        JobID jobID;
        JobTypeID jobTypeID;

        Job(JobID jobID = 0);
        void restartJob();
        void addOperation(const Operation& operation);
        void addOperations(const std::vector<Operation> operations);
        bool dumpOperation(OperationID operationID, int endTime);
        const Operation& getOperation(OperationID operationID) const;
        bool isDone() const;
        const std::vector<OperationID>& getAvailibleOperations() const { return m_availibleOperations; }
        const std::map <OperationID, Operation>& getOperaions() const { return m_operations; }
        const std::vector<OperationID>& getSuccessors(OperationID operationID) const;
        float remainingAverageProcessTime() const;
        float avgTimeBlockedByOperation(OperationID operationID) const;

        float nc_criticalPath() const;
        float eft(OperationID operationID) const;
        float criticalPath(OperationID operationID) const;

        int calculateLF(OperationID operation, const Schedule& schedule) const;
        int calculateES(OperationID operation, const Schedule& schedule) const;

        int getSuccessorsUpstream(OperationID operation) const;
        int remainingNumOfOperations() const;
    };

    /* Job container class ==================================== */
    class JobContainer {
        std::map<JobID, Job> m_jobs;
        int m_stationCount;

    public:
        void addJob(const Job& job);
        void addJobs(const std::vector<Job>& jobs);
        bool isDone() const;
        Job& getJob(JobID jobID);
        const Job& getJob(JobID jobID) const;
        std::map<JobID, Job>& getJobs() { return m_jobs; }
        const std::map<JobID, Job>& getJobs() const { return m_jobs; }
        void restartContainer();
        int stationCount() const;
        void setStationCount(int stationCount);
    };

    /* ScheduledOperation class ==================================== */
    struct ScheduledOperation {
        ScheduledOperation() = default;
        ScheduledOperation(JobID jobID, 
            JobTypeID jobTypeID, 
            OperationID operationID, 
            OperationTypeID operationTypeID, 
            StationID stationID,
            int startTime, 
            int duration);

        JobID jobID;
        JobTypeID jobTypeID;
        OperationID operationID;
        OperationTypeID operationTypeID;
        StationID stationID;
        int startTime;
        int duration;

        int endTime() const;
    };

    /* Schedule class ==================================== */
    class Schedule {
        std::vector<std::vector<ScheduledOperation>> m_schedule; // stationID, operationScheduled

    public:
        Schedule(size_t stationsCount);
        int fastestTimeForScheduleOperation(StationID stationID, OperationID operationID, JobID jobID, const JobContainer& jobContainer) const;
        int fastestEndTimeForScheduleOperation(StationID stationID, OperationID operationID, JobID jobID, const JobContainer& jobContainer) const;
        ScheduledOperation stackScheduleOperation(StationID stationID, OperationID operationID, JobID jobID, JobContainer& jobContainer);
        int getStationAvabilityTime(StationID stationID) const;
        void print() const;
        int makeSpan() const;
        void clear();
        std::vector<std::vector<ScheduledOperation>>& getSchedule();
        int stationCount() const;
    };
}