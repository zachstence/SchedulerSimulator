/**
 * @author zachstence / zms22
 * @since 3/29/2019
 */

#ifndef CS4328_PROJECT1_READYQUEUE_H
#define CS4328_PROJECT1_READYQUEUE_H

#include "Process.h"

/**
 * A class to hold Processes in a queue-like structure using a specified priority comparator to control the processes'
 * ordering.
 * @tparam PriorityComparator The comparator to use for sorting the processes in the queue. Must implement operator()
 * to specify the priority scheme.
 */
template <class PriorityComparator>
class ReadyQueue {

public:

    /**
     * Adds a process to the ReadyQueue in the correct position based on its priority.
     * @param p The process to add to the ReadyQueue.
     */
    void add(Process* p) {
        processes.insert(p);
    }

    /**
     * Gets the process in the front of the ReadyQueue and removes it from the ReadyQueue.
     * @return The process that was at the front of the ReadyQueue.
     */
    Process* getFront() {
        Process* p = *processes.begin();
        processes.erase(p);
        return p;
    }

    /**
     * Returns the processes in the ReadyQueue as a set.
     * @return The processes in the ReadyQueue.
     */
    const set<Process*, PriorityComparator> getProcesses() {
        return processes;
    }

    /**
     * Updates the wait time of each process in the ReadyQueue.
     * @param clock The current clock time (in seconds).
     */
    void updateWaitTimes(double clock) {
        for (auto process : this->getProcesses()) {
            // If process has never been on CPU, its wait time correponds to its arrival time
            if (process->getLastTimeAssignedCpu() == -1)
                process->setWaitTime(clock - process->getArrivalTime());
            // Otherwise its wait time corresponds to the last time it was assigned the CPU
            else
                process->setWaitTime(clock - process->getLastTimeAssignedCpu());
        }
    }

    /**
     * Resorts the ReadyQueue. Intended to be used after updating priority values of processes in the ReadyQueue.
     */
    void sort() {
        processes = set<Process*, PriorityComparator>(processes.begin(), processes.end());
    }

    /**
     * Determines whether the ReadyQueue is empty (has no processes in it).
     * @return True if the ReadyQueue is empty, false otherwise.
     */
    bool empty() {
        return processes.empty();
    }

    /**
     * Returns the size of the ReadyQueue.
     * @return The number of processes in the ReadyQueue.
     */
    unsigned long size() {
        return processes.size();
    }

private:
    set<Process*, PriorityComparator> processes;

};


#endif //CS4328_PROJECT1_READYQUEUE_H
