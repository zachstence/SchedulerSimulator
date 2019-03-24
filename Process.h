//
// Created by zach on 3/18/19.
//

#ifndef CS4328_PROJECT1_PROCESS_H
#define CS4328_PROJECT1_PROCESS_H

#include <cmath>

class Process {
private:
    int id;
    double arrivalTime;
    double serviceTime;

    double waitTime;
    double lastTimeAssignedCpu;
    double serviceTimeLeft;
    double completionTime;

public:
    Process(int id, double arrivalTime, double serviceTime) {
        this->id = id;
        this->arrivalTime = arrivalTime;
        this->serviceTime = serviceTime;

        this->waitTime = 0;
        this->lastTimeAssignedCpu = -1;
        this->serviceTimeLeft = serviceTime;
        this->completionTime = -1;
    }

    int getId() { return this->id; }

    double getArrivalTime() { return this->arrivalTime; }

    double getServiceTime() { return this->serviceTime; }

    double getWaitTime() { return this->waitTime; }
    double setWaitTime(double waitTime) { this->waitTime = waitTime; }

    double getLastTimeAssignedCpu() { return this->lastTimeAssignedCpu; }
    double setLastTimeAssignedCpu(double lastTimeOnCpu) { this->lastTimeAssignedCpu = lastTimeOnCpu; }

    double getServiceTimeLeft() { return this->serviceTimeLeft; }
    double setServiceTimeLeft(double serviceTimeLeft) { this->serviceTimeLeft = serviceTimeLeft; }

    void setCompletionTime(double completionTime) { this->completionTime = completionTime; }
    double getCompletionTime() { return this->completionTime; }

    double calcResponseRatio() {
        return (this->waitTime + this->serviceTime) / this->serviceTime;
    }

};


#endif //CS4328_PROJECT1_PROCESS_H
