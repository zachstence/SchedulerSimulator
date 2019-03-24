//
// Created by zach on 3/19/19.
//

#ifndef CS4328_PROJECT1_READYQUEUE_H
#define CS4328_PROJECT1_READYQUEUE_H

#include <string>
#include <list>

#include "Process.h"

using std::string;
using std::list;

template <class PriorityComparator>
class ReadyQueue {
private:
    set<Process*, PriorityComparator> processes;

public:

    void add(Process* p) {
        auto result = processes.insert(p);
        if (!result.second)
            printf("Error adding process %d to ready queue\n", p->getId());
    }

    Process* getFront() {
        Process* p = *processes.begin();
        processes.erase(p);
        return p;
    }

    const set<Process*, PriorityComparator> getProcesses() {
        return processes;
    }

    void updateWaitTimes(double clock) {
        for (auto process : this->getProcesses()) {
            if (process->getLastTimeAssignedCpu() == -1)
                process->setWaitTime(clock - process->getArrivalTime());
            else
                process->setWaitTime(clock - process->getLastTimeAssignedCpu());
        }
        this->sort();
    }

    void sort() {
        processes = set<Process*, PriorityComparator>(processes.begin(), processes.end());
    }

    bool empty() {
        return processes.empty();
    }

    unsigned long size() {
        return processes.size();
    }

    string to_string() {
        string out;
        for (auto p : processes) {
            out += (std::to_string(p->getId()) + "\t: " +
                    std::to_string(p->getArrivalTime()) + "\t" +
                    std::to_string(p->getServiceTime()) +
                    "\n"
            );
        }
        return out;
    }
};


#endif //CS4328_PROJECT1_READYQUEUE_H
