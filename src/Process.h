/**
 * @author zachstence / zms22
 * @since 3/29/2019
 */

#ifndef CS4328_PROJECT1_PROCESS_H
#define CS4328_PROJECT1_PROCESS_H

#include <cmath>

/**
 * A class to hold information about a process.
 */
class Process {

public:

    /**
     * Creates a Process object given an ID, arrival time, and service time.
     * @param id The ID of the process.
     * @param arrivalTime The arrival time (in seconds) of the process.
     * @param serviceTime The service time (in seconds) of the process.
     */
    Process(int id, double arrivalTime, double serviceTime) {
        this->id = id;
        this->arrivalTime = arrivalTime;
        this->serviceTime = serviceTime;

        this->waitTime = 0;
        this->lastTimeAssignedCpu = -1;
        this->serviceTimeLeft = serviceTime;
        this->completionTime = -1;
    }

    /**
     * Returns the ID of this process.
     * @return The ID of this process.
     */
    int getId() { return this->id; }

    /**
     * Returns the arrival time of this process.
     * @return The arrival time (in seconds) of this process.
     */
    double getArrivalTime() { return this->arrivalTime; }

    /**
     * Returns the service time of this process.
     * @return The service time (in seconds) of this process.
     */
    double getServiceTime() { return this->serviceTime; }

    /**
     * Returns the current wait time of this process.
     * @return The current wait time (in seconds) of this process.
     */
    double getWaitTime() { return this->waitTime; }

    /**
     * Sets the current wait time of this process.
     * @param waitTime The new wait time (in seconds) of this process.
     */
    void setWaitTime(double waitTime) { this->waitTime = waitTime; }

    /**
     * Returns the last time this process was assigned to the CPU.
     * @return The last time (in seconds) this process was assigned to the CPU.
     */
    double getLastTimeAssignedCpu() { return this->lastTimeAssignedCpu; }

    /**
     * Sets the last time this process was assigned the CPU.
     * @param lastTimeOnCpu The time (in seconds) this process was assigned the CPU.
     */
    void setLastTimeAssignedCpu(double lastTimeOnCpu) { this->lastTimeAssignedCpu = lastTimeOnCpu; }

    /**
     * Returns the service time this process has left.
     * @return The service time (in seconds) this process has left.
     */
    double getServiceTimeLeft() { return this->serviceTimeLeft; }

    /**
     * Sets the amount of time left this process needs on the CPU.
     * @param serviceTimeLeft The amount of time (in seconds) this process still needs on the CPU.
     */
    void setServiceTimeLeft(double serviceTimeLeft) { this->serviceTimeLeft = serviceTimeLeft; }

    /**
     * Returns the completion time of this process.
     * @return The completion time (in seconds) of this process.
     */
    double getCompletionTime() { return this->completionTime; }

    /**
     * Sets the completion time of the process
     * @param completionTime The completion time (in seconds) of the process.
     */
    void setCompletionTime(double completionTime) { this->completionTime = completionTime; }

    /**
     * Calculates the response ratio of the process, used in the HRRN scheduler.
     * @return The response ratio of the process.
     */
    double calcResponseRatio() {
        return (this->waitTime + this->serviceTime) / this->serviceTime;
    }

private:
    int id;
    double arrivalTime;
    double serviceTime;

    double waitTime;
    double lastTimeAssignedCpu;
    double serviceTimeLeft;
    double completionTime;

};


#endif //CS4328_PROJECT1_PROCESS_H
