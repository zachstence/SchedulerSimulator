/**
 * @author zachstence / zms22
 * @since 3/24/2019
 */

#ifndef CS4328_PROJECT1_PRIORITYCOMPARATOR_H
#define CS4328_PROJECT1_PRIORITYCOMPARATOR_H

#include "Process.h"

/**
 * An abstract base class for priority comparators for use in priority based scheduling simulations.
 */
class PriorityComparator {
    /**
     * A pure virtual function that templates the comparison of two processes.
     * @param p1 A pointer to a Process.
     * @param p2 A pointer to a Process.
     * @return True if p1 points to a process with higher priority than p2, false otherwise.
     */
    virtual bool operator()(Process* p1, Process* p2) = 0;
};

/**
 * A PriorityComparator that uses the First Come First Serve (FCFS) priority scheme. A process has higher priority than another
 * if its arrival time is sooner.
 */
class FCFSPriorityComparator : PriorityComparator {
public:
    /**
     * Compares the priority of two processes under First Come First Serve (FCFS).
     * @param p1 A pointer to a Process.
     * @param p2 A pointer to a Process.
     * @return True if p1 points to a process with higher priority than p2, false otherwise.
     */
    bool operator()(Process* p1, Process* p2) override {
        return p1->getArrivalTime() < p2->getArrivalTime();
    }
};

/**
 * A PriorityComparator that uses the Shortest Remaining Time First (SRTF) priority scheme. A process has higher priority than
 * another if the amount of service time it has left is smaller.
 */
class SRTFPriorityComparator : PriorityComparator {
public:
    /**
     * Compares the priority of two processes under Shortest Remaining Time First (SRTF).
     * @param p1 A pointer to a Process.
     * @param p2 A pointer to a Process.
     * @return True if p1 points to a process with higher priority than p2, false otherwise.
     */
    bool operator()(Process* p1, Process* p2) override {
        if (p1->getServiceTimeLeft() != p2->getServiceTimeLeft())
            return p1->getServiceTimeLeft() < p2->getServiceTimeLeft();
        else
            return p1->getArrivalTime() < p2->getArrivalTime();
    }
};

/**
 * A PriorityComparator that uses the Highest Response Ratio Next (HRRN) priority scheme. A process has higher priority than
 * another if its response ratio is greater.
 */
class HRRNPriorityComparator : PriorityComparator {
public:
    /**
     * Compares the priority of two processes under Highest Response Ratio Next (HRRN).
     * @param p1 A pointer to a Process.
     * @param p2 A pointer to a Process.
     * @return True if p1 points to a process with higher priority than p2, false otherwise.
     */
    bool operator()(Process* p1, Process* p2) override {
        if (p1->calcResponseRatio() != p2->calcResponseRatio())
            return p1->calcResponseRatio() > p2->calcResponseRatio();
        else
            return p1->getArrivalTime() < p2->getArrivalTime();
    }
};

#endif //CS4328_PROJECT1_PRIORITYCOMPARATOR_H
