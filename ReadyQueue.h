//
// Created by zach on 3/19/19.
//

#ifndef CS4328_PROJECT1_READYQUEUE_H
#define CS4328_PROJECT1_READYQUEUE_H

#include <string>
#include <set>

#include "Process.h"

using std::string;
using std::set;

template <class PriorityComparator>
class ReadyQueue {
private:
    set<Process*, PriorityComparator> processes;

public:

    void add(Process* p) {
        processes.insert(p);
    }

    Process* getFront() {
        Process* p = *processes.begin();
        processes.erase(p);
        return p;
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
