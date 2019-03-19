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

class HRRNPriorityComparator : PriorityComparator {
public:
    bool operator()(Process* p1, Process* p2) {
        return p1->calcResponseRatio() > p2->calcResponseRatio();
    }
};

#endif //CS4328_PROJECT1_PRIORITYCOMPARATOR_H
