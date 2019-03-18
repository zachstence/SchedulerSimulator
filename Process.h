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

    double lastTimeOnCpu;
    double serviceTimeLeft;
    double completionTime;

public:
    Process() {
        this->id = -1;
        this->arrivalTime = -1;
        this->serviceTime = -1;
    }
    Process(int id, double arrivalTime, double serviceTime) {
        this->id = id;
        this->arrivalTime = arrivalTime;
        this->serviceTime = serviceTime;

        this->lastTimeOnCpu = -1;
        this->serviceTimeLeft = serviceTime;
        this->completionTime = -1;
    }

    void decreaseServiceTimeLeft(double amount) {
        this->serviceTimeLeft -= amount;
        // TODO remove later?
        if (this->serviceTimeLeft <= 0) printf("Scheduling error! Process used more CPU than needed!\n");
    }

    int getId() { return this->id; }

    double getArrivalTime() { return this->arrivalTime; }

    double getServiceTime() { return this->serviceTime; }

    double getLastTimeOnCpu() { return this->lastTimeOnCpu; }
    double setLastTimeOnCpu(double lastTimeOnCpu) { this->lastTimeOnCpu = lastTimeOnCpu; }

    double getServiceTimeLeft() { return this->serviceTimeLeft; }

    void setCompletionTime(double completionTime) { this->completionTime = completionTime; }
    double getCompletionTime() { return this->completionTime; }

};


#endif //CS4328_PROJECT1_PROCESS_H
