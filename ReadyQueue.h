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
//    list<Process*> processes;

public:

    void add(Process* p) {
        auto result = processes.insert(p);
        if (!result.second)
            printf("Error adding process %d to ready queue\n", p->getId());
    }

/*
    void add(Process* p) {
        if (processes.empty()) {
            processes.insert(processes.begin(), p);
            return;
        }
        auto it = processes.begin();
        while (it != processes.end()) {
            if (PriorityComparator()(p, *it))
                processes.insert(it, p);
            it++;
        }
        processes.insert(it, p);
    }
*/

    Process* getFront() {
        Process* p = *processes.begin();
        processes.erase(p);
        return p;
    }

/*
    Process* getFront() {
        Process* p = *processes.begin();
        processes.erase(processes.begin());
        return p;
    }
*/

    const set<Process*, PriorityComparator> getProcesses() {
        return processes;
    }


    void sort() {
//        std::sort(processes.begin(), processes.end(), PriorityComparator());
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
