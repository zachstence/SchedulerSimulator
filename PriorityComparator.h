//
// Created by zach on 3/19/19.
//

#ifndef CS4328_PROJECT1_PRIORITYCOMPARATOR_H
#define CS4328_PROJECT1_PRIORITYCOMPARATOR_H

#include "Process.h"

class PriorityComparator {
    virtual bool operator()(Process* p1, Process* p2) = 0;
};

class FCFSPriorityComparator : PriorityComparator {
public:
    bool operator()(Process* p1, Process* p2) {
        return p1->getArrivalTime() < p2->getArrivalTime();
    }
};

class SRTFPriorityComparator : PriorityComparator {
public:
    bool operator()(Process* p1, Process* p2) {
        if (p1->getServiceTimeLeft() != p2->getServiceTimeLeft())
            return p1->getServiceTimeLeft() < p2->getServiceTimeLeft();
        else
            return p1->getArrivalTime() < p2->getArrivalTime();
    }
};

class HRRNPriorityComparator : PriorityComparator {
public:
    bool operator()(Process* p1, Process* p2) {
        if (p1->calcResponseRatio() != p2->calcResponseRatio())
            return p1->calcResponseRatio() > p2->calcResponseRatio();
        else
            return p1->getArrivalTime() < p2->getArrivalTime();
    }
};

#endif //CS4328_PROJECT1_PRIORITYCOMPARATOR_H
