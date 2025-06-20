#include "FJSS.hpp"

namespace fjss {
    Operation::Operation() {
        operationID = 0;
        operationTypeID = 0;
        m_lastPrecedessorTime = 0;
        m_done = false;
    }

    void Operation::addOperationTimeStation(const OperationTimeStation& operationTimeStation) {
        m_operationTimeStations.push_back(operationTimeStation);
    };

    void Operation::addPredecessor(OperationID operationID) {
        m_predecessors.push_back(operationID);
    }

    void Operation::restartOperation() {
        m_predecessorsToDo.clear();
        for (const auto& it : m_predecessors) m_predecessorsToDo.insert(it);
        m_lastPrecedessorTime = 0;
        m_done = false;
    }

    bool Operation::isAvailible() const {
        return m_predecessorsToDo.empty();
    }

    void Operation::precedessordoneUpdate(OperationID predecessorID, int time) {
        m_predecessorsToDo.erase(predecessorID);
        m_lastPrecedessorTime = std::max(m_lastPrecedessorTime, time);
    }

    int Operation::getLastPrecedessorTime() const {
        return m_lastPrecedessorTime;
    }

    int Operation::getProcessTimeOnStationID(StationID stationID) const {
        for (const auto& operationTimeStation : m_operationTimeStations) {
            if (operationTimeStation.stationID == stationID) {
                return operationTimeStation.time;
            }
        }
        throw std::runtime_error("Operation can't be processed on this station");
    }

    void Operation::markDone() {
        m_done = true;
    }

    bool Operation::isDone() const {
        return m_done;
    }

    float Operation::averageProcessTime() const {
        float avg = 0;
        for (const auto& operationTimeStation : m_operationTimeStations) {
            avg += operationTimeStation.time;
        }
        return avg / (float)m_operationTimeStations.size();
    }

    int Operation::getShortestProcessTime() const {
        int shortestTime = std::numeric_limits<int>::max();
        for (const auto& operationTimeStation : m_operationTimeStations) {
            shortestTime = std::min(shortestTime, operationTimeStation.time);
        }
        return shortestTime;
    }

    int Operation::getLongestProcessTime() const
    {
        int longestTime = 0;
        for (const auto& operationTimeStation : m_operationTimeStations) {
            longestTime = std::max(longestTime, operationTimeStation.time);
        }
        return longestTime;
    }



    //---------------------

    Job::Job(JobID jobID) {
        this->jobID = jobID;
        jobTypeID = 0;
    }

    void Job::restartJob() {
        m_availibleOperations.clear();
        for (auto& it : m_operations) {
            it.second.restartOperation();
            if (it.second.isAvailible()) m_availibleOperations.push_back(it.first);
        }
    }

    void Job::addOperation(const Operation& operation) {
        m_operations.insert({ operation.operationID, operation });
        if (m_successors.count(operation.operationID) == 0) {
            m_successors.insert({ operation.operationID, std::vector<OperationID>() });
        }
        for (OperationID predecessorOpID : operation.getPredecessors()) {
            m_successors[predecessorOpID].push_back(operation.operationID);
        }
    }

    void Job::addOperations(const std::vector<Operation> operations) {
        for (const Operation& operation : operations) addOperation(operation);
    }

    bool Job::dumpOperation(OperationID operationID, int endTime) {
        auto idItr = std::find(m_availibleOperations.begin(), m_availibleOperations.end(), operationID);
        if (idItr == m_availibleOperations.end()) {
            return false;
        }
        m_availibleOperations.erase(idItr);
        m_operations[operationID].markDone();
        // infrom all succesors and check their avibility
        for (auto& successorOpID : m_successors[operationID]) {
            m_operations[successorOpID].precedessordoneUpdate(operationID, endTime);
            if (m_operations[successorOpID].isAvailible()) {
                m_availibleOperations.push_back(successorOpID);
            }
        }
        return true;
    }

    const Operation& Job::getOperation(OperationID operationID) const {
        return m_operations.at(operationID);
    }

    bool Job::isDone() const {
        return m_availibleOperations.empty();
    }

    const std::vector<OperationID>& Job::getSuccessors(OperationID operationID) const {
        return m_successors.at(operationID);
    }

    float Job::remainingAverageProcessTime() const
    {
        float avg = 0;
        for (const auto& it : m_operations) {
            if (it.second.isDone()) continue;
            avg += it.second.averageProcessTime();
        }
        return avg;
    }

    float Job::avgTimeBlockedByOperation(OperationID operationID) const
    {
        float avgTimeBlocked = 0;
        const std::vector<OperationID>& successors = getSuccessors(operationID);
        if (successors.empty()) return 0;
        for (const OperationID& successorID : successors) {
            avgTimeBlocked += m_operations.at(successorID).averageProcessTime();
            avgTimeBlocked += avgTimeBlockedByOperation(successorID);
        }
        return avgTimeBlocked;
    }

    float Job::nc_criticalPath() const
    {
        // find all ending operation
        int operationCount = m_operations.size();
        float maxEnd = -1.0;
        for (OperationID i = 0; i < operationCount; ++i) {
            if (m_successors.at(i).size() == 0) {
                maxEnd = std::max(eft(i), maxEnd);
            }
        }
        return maxEnd;
    }

    float Job::eft(OperationID operationID) const
    {
        const Operation& operation = m_operations.at(operationID);
        if (operation.isDone() || operation.getPredecessors().size() == 0) {
            return operation.getLastPrecedessorTime() + operation.getShortestProcessTime();
        }
        const std::vector<OperationID>& predecessorOperations = operation.getPredecessors();
        float maxEnd = -1.0;
        for (const OperationID& predecessorID : predecessorOperations) {
            maxEnd = std::max(maxEnd, eft(predecessorID));
        }
        return maxEnd + operation.getShortestProcessTime();
    }

    float Job::criticalPath(OperationID operationID) const
    {
        const Operation& operation = m_operations.at(operationID);
        const std::vector<OperationID> successors = m_successors.at(operationID);
        float cpl = 0;
        for (const OperationID successorID : successors) {
            cpl = std::max(cpl, criticalPath(successorID));
        }
        return cpl + operation.getShortestProcessTime();
        //return cpl + operation.getLongestProcessTime();
    }

    int Job::calculateLF(OperationID operationID, const Schedule& schedule) const
    {
        const Operation& operation = m_operations.at(operationID);
        const std::vector<OperationID> successors = m_successors.at(operationID);
        if (successors.empty()) {
            return schedule.makeSpan();
        }
        int minLS = std::numeric_limits<int>::max();
        for (const OperationID successorID : successors) {
            const Operation& succ_operation = m_operations.at(successorID);
            int successorLS = calculateLF(successorID, schedule) - succ_operation.getShortestProcessTime();
            minLS = std::min(minLS, successorLS);
        }
        return minLS;
    }

    int Job::calculateES(OperationID operationID, const Schedule& schedule) const
    {
        const Operation& operation = m_operations.at(operationID);
        const std::vector<OperationID>& predecessors = operation.getPredecessors();

        int maxEF = operation.getLastPrecedessorTime();
        for (const OperationID predecessorID : predecessors) {
            const Operation& pred_operation = m_operations.at(predecessorID);
            if (pred_operation.isDone()) continue;
            int predecessorEF = calculateES(predecessorID, schedule) + pred_operation.getShortestProcessTime();
            maxEF = std::max(maxEF, predecessorEF);
        }

        int pt = operation.getShortestProcessTime();
        for (const OperationTimeStation& ots : operation.getOperationTimeStations()) {
            if (pt == ots.time) {
                return std::max(schedule.getStationAvabilityTime(ots.stationID), maxEF);
            }
        }
    }

    int Job::getSuccessorsUpstream(OperationID operationID) const
    {
        const Operation& operation = m_operations.at(operationID);
        const std::vector<OperationID> successors = m_successors.at(operationID);
        if (successors.empty()) return 0;
        int suc = 0;
        for (const OperationID successorID : successors) {
            suc = suc + getSuccessorsUpstream(successorID) + 1;
        }
        return suc;
    }

    int Job::remainingNumOfOperations() const
    {
        int remaining = 0;
        for (const auto& it : m_operations) {
            if (it.second.isDone()) continue;
            ++remaining;
        }
        return remaining;
    }



    //------------------------------

    void JobContainer::addJob(const Job& job) {
        m_jobs.insert({ job.jobID, job });
    }

    void JobContainer::addJobs(const std::vector<Job>& jobs) {
        for (const Job& job : jobs) m_jobs.insert({ job.jobID, job });
    }

    bool JobContainer::isDone() const {
        for (const auto& job : m_jobs) {
            if (job.second.isDone() == false) return false;
        }
        return true;
    }

    Job& JobContainer::getJob(JobID jobID) {
        return m_jobs.at(jobID);
    }

    const Job& JobContainer::getJob(JobID jobID) const
    {
        return m_jobs.at(jobID);
    }

    void JobContainer::restartContainer() {
        for (auto& jobIT : m_jobs) {
            jobIT.second.restartJob();
        }
    }

    int JobContainer::stationCount() const
    {
        return m_stationCount;
    }

    void JobContainer::setStationCount(int stationCount)
    {
        m_stationCount = stationCount;
    }



    // -----------------------------------------------


    ScheduledOperation::ScheduledOperation(JobID jobID,
        JobTypeID jobTypeID,
        OperationID operationID,
        OperationTypeID operationTypeID,
        StationID stationID,
        int startTime,
        int duration) {
        this->jobID = jobID;
        this->jobTypeID = jobTypeID;
        this->operationID = operationID;
        this->operationTypeID = operationTypeID;
        this->startTime = startTime;
        this->duration = duration;
        this->stationID = stationID;
    }

    int ScheduledOperation::endTime() const {
        return startTime + duration;
    }


    // ---------------------------------------



    Schedule::Schedule(size_t stationsCount) {
        m_schedule.resize(stationsCount);
    }

    int Schedule::fastestTimeForScheduleOperation(StationID stationID, OperationID operationID, JobID jobID, const JobContainer& jobContainer) const {
        if (stationID >= m_schedule.size()) {
            throw std::runtime_error("No such station");
        }
        const Job& job = jobContainer.getJob(jobID);
        const Operation& operation = job.getOperation(operationID);
        int lastPrecedessorTime = operation.getLastPrecedessorTime();
        int stationAvability = getStationAvabilityTime(stationID);
        return std::max(lastPrecedessorTime, stationAvability);
    }

    int Schedule::fastestEndTimeForScheduleOperation(StationID stationID, OperationID operationID, JobID jobID, const JobContainer& jobContainer) const {
        int fastestTimeForScheduleOp = fastestTimeForScheduleOperation(stationID, operationID, jobID, jobContainer);
        const Job& job = jobContainer.getJob(jobID);
        const Operation& operation = job.getOperation(operationID);
        return fastestTimeForScheduleOp + operation.getProcessTimeOnStationID(stationID);
    }

    ScheduledOperation Schedule::stackScheduleOperation(StationID stationID, OperationID operationID, JobID jobID, JobContainer& jobContainer) {
        Job& job = jobContainer.getJob(jobID);
        const Operation& operation = job.getOperation(operationID);
        int startTime = fastestTimeForScheduleOperation(stationID, operationID, jobID, jobContainer);
        int duration = operation.getProcessTimeOnStationID(stationID);
        ScheduledOperation sop(jobID, job.jobTypeID, operationID, operation.operationTypeID, stationID, startTime, duration);
        m_schedule[stationID].push_back(sop);
        job.dumpOperation(operationID, sop.endTime());
        return sop;
    }

    int Schedule::getStationAvabilityTime(StationID stationID) const {
        if (stationID >= m_schedule.size()) {
            throw std::runtime_error("No such station");
        }
        if (m_schedule[stationID].size() == 0) {
            return 0;
        }
        return m_schedule[stationID].back().endTime();
    }

    void Schedule::print() const {
        int stationCount = m_schedule.size();
        for (StationID stationID = 0; stationID < stationCount; ++stationID) {
            std::cout << "S" << stationID << ": ";
            for (const ScheduledOperation& sop : m_schedule[stationID]) {
                std::cout << "o" << sop.operationID << "(J" << sop.jobID << ")[" << sop.startTime << "-" << sop.endTime() << "], ";
            }
            std::cout << std::endl << std::endl;
        }
        std::cout << "MakeSpan (Cmax): " << makeSpan() << "\n";
    }

    int Schedule::makeSpan() const {
        int makeSpan = 0;
        for (const auto& station : m_schedule) {
            if (station.empty()) continue;
            makeSpan = std::max(makeSpan, station.back().endTime());
        }
        return makeSpan;
    }

    void Schedule::clear()
    {
        for (std::vector<ScheduledOperation>& machineVec : m_schedule) {
            machineVec.clear();
        }
    }

    std::vector<std::vector<ScheduledOperation>>& Schedule::getSchedule()
    {
        return m_schedule;
    }
    int Schedule::stationCount() const
    {
        return m_schedule.size();
    }
}
