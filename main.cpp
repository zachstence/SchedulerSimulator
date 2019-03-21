#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>

#include <list>
#include <cfloat>

using std::list;

#include "EventQueue.h"
#include "ReadyQueue.h"
#include "PriorityComparator.h"

using std::string;
using std::to_string;
using std::cout;
using std::endl;
using std::ofstream;
using std::stoi;
using std::stod;
using std::vector;
using std::priority_queue;
using std::queue;
using std::find;


struct Statistics {
    double avgTurnaroundTime;
    double throughput;
    double avgCpuUtil;
    double avgReadyQueueSize;

    void display() {
        printf("Avg. Turnaround Time : %6.3f\n"
               "          Throughput : %6.3f\n"
               "       Avg. CPU Util : %6.3f\n"
               " Avg. in Ready Queue : %6.3f\n",
               this->avgTurnaroundTime, this->throughput, this->avgCpuUtil, this->avgReadyQueueSize
        );
    }
};

double inversePoisson(double rate) {
    double y = (double) rand() / RAND_MAX;
    return -1 * log(1 - y) / rate;
}

// TODO maybe put all code for choosing next process into its own function? would simplify simulatePriorityBased a lot
// TODO and then whether its an arrival causing preemption, a departure, or a timeout preemption just call the same function
// TODO to decide what process to run next. Would need to have readyQueue, doPreemption, and dynamicPriorty and comparator
// TODO template as arguments to make scheduling decision. Wouldn't need to return anything, just have it modify the data structures
// TODO might make code even more modular and remove duplicated stuff

// TODO probably add parameter to simulatePriorityBased that is an interval for round robin-ing
// TODO if negative, don't do RR, otherwise use that number as interval in seconds

template <class PriorityComparator>
Statistics simulatePriorityBased(int numProcesses, double arrivalRate, double serviceTime, double quantumLength,
                                 double queryInterval, bool doPreemption, bool dynamicPriority) {
    EventQueue eventQueue;
    ReadyQueue<PriorityComparator> readyQueue;
    vector<Process*> processes;
    Process* onCpu = nullptr;

    double clock = 0;
    bool cpuIdle = true;

    // Schedule first process arrival
    auto firstProcess = new Process(0, 0, inversePoisson(1 / serviceTime));
    processes.push_back(firstProcess);
    eventQueue.scheduleEvent(clock, firstProcess, ARRIVAL);

    // Schedule first timeout event for round robin (if quantumLength isn't negative or 0)
    if (quantumLength >= 0)
        eventQueue.scheduleEvent(clock + quantumLength, nullptr, TIMEOUT);

    // Schedule first query event
    eventQueue.scheduleEvent(clock + queryInterval, nullptr, QUERY);

    int processesSimulated = 0;
    double cpuIdleTime = 0;
    double lastCpuBusyTime = 0;
    int totalInReadyQueue = 0;

    while (processesSimulated < numProcesses && !eventQueue.empty()) {

        Event current = eventQueue.getEvent();
        clock = current.time;

        // If arrival event
        if (current.type == ARRIVAL) {
            // If CPU is idle
            if (cpuIdle) {
                // Set CPU to busy and update idle time
                cpuIdle = false;
                cpuIdleTime += clock - lastCpuBusyTime;
                // Assign arriving process to CPU
                current.process->setLastTimeAssignedCpu(clock);
                onCpu = current.process;
                // Schedule arriving process' tentative departure
                eventQueue.scheduleEvent(clock + current.process->getServiceTimeLeft(), current.process, DEPARTURE);
            }
            // If CPU is busy, add arriving process to ready queue and compare onCpu with front of ready queue for preemption
            else {
                // Add arriving process to ready queue and update statistics
                readyQueue.add(current.process);
                if (dynamicPriority)
                    readyQueue.updateWaitTimes(clock);

                if (doPreemption) {
                    onCpu->setServiceTimeLeft(onCpu->getServiceTimeLeft() - (clock - onCpu->getLastTimeAssignedCpu()));
                    readyQueue.updateWaitTimes(clock);

                    Process* candidate = readyQueue.getFront();
                    if (PriorityComparator()(candidate, onCpu)) {
                        // Delete tentative departure of process on CPU
                        eventQueue.unscheduleDeparture(onCpu->getId());
                        // Move process from CPU to ready queue
                        readyQueue.add(onCpu);
                        // Assign arriving process to CPU
                        onCpu = candidate;
                        // Schedule tentative departure for new process
                        eventQueue.scheduleEvent(clock + onCpu->getServiceTimeLeft(), onCpu, DEPARTURE);
                    }
                    else {
                        readyQueue.add(candidate);
                    }
                    // Update last time assigned CPU
                    onCpu->setLastTimeAssignedCpu(clock);
                }
            }

            // Schedule next arrival
            int nextId = current.process->getId() + 1;
            double nextArrivalTime = clock + inversePoisson(arrivalRate);
            auto nextArrival = new Process(nextId, nextArrivalTime, inversePoisson(1 / serviceTime));
            processes.push_back(nextArrival);
            eventQueue.scheduleEvent(nextArrivalTime, nextArrival, ARRIVAL);
        }
        // If departure event
        else if (current.type == DEPARTURE) {
            // Increment number of processes simulated at each departure
            processesSimulated++;
            current.process->setCompletionTime(clock);

            // If ready queue is empty, set CPU to idle
            if (readyQueue.empty()) {
                cpuIdle = true;
                onCpu = nullptr;
                lastCpuBusyTime = clock;
            }
            // If ready queue is not empty, put next process from ready queue on CPU
            // and schedule its tentative departure
            else {
                // Update wait times and resort queue
                if (dynamicPriority)
                    readyQueue.updateWaitTimes(clock);

                Process* p = readyQueue.getFront();
                p->setLastTimeAssignedCpu(clock);
                onCpu = p;
                eventQueue.scheduleEvent(clock + p->getServiceTimeLeft(), p, DEPARTURE);
            }
        }
        else if (current.type == TIMEOUT) {
            if (cpuIdle) {
                break;
            }
            else {
                onCpu->setServiceTimeLeft(onCpu->getServiceTimeLeft() - (clock - onCpu->getLastTimeAssignedCpu()));
                eventQueue.unscheduleDeparture(onCpu->getId());
                readyQueue.add(onCpu);
                if (dynamicPriority)
                    readyQueue.updateWaitTimes(clock);
                onCpu = readyQueue.getFront();
                eventQueue.scheduleEvent(clock + onCpu->getServiceTimeLeft(), onCpu, DEPARTURE);
                onCpu->setLastTimeAssignedCpu(clock);
            }
            // Schedule next timeout
            eventQueue.scheduleEvent(clock + quantumLength, nullptr, TIMEOUT);
        }
        else if (current.type == QUERY) {
            totalInReadyQueue += readyQueue.size();
            eventQueue.scheduleEvent(clock + queryInterval, nullptr, QUERY);
        }
    }

    // Sum turnaround times for statistics
    double totalTurnaroundTime = 0;
    for (auto process : processes) {
        if (process->getCompletionTime() == -1) continue;
        double turnaround = process->getCompletionTime() - process->getArrivalTime();
        totalTurnaroundTime += turnaround;
    }

    double avgTurnaroundTime = totalTurnaroundTime / numProcesses;
    double throughput = processesSimulated / clock;
    double avgCpuUtil = 1 - (cpuIdleTime / clock);
    double avgReadyQueueSize = (double) totalInReadyQueue / (clock / queryInterval);

    return {avgTurnaroundTime, throughput, avgCpuUtil, avgReadyQueueSize};

}

void runAllSimulations() {

    vector<double> arrivalRates {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                                 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};

    int numProcesses = 10000;
    double serviceTime = 0.06;
    double queryInterval = 0.01;

    Statistics s {};
    ofstream csvOut;

/*
    // FCFS
    csvOut.open("FCFS.csv");
    for (double arrivalRate : arrivalRates) {
        cout << "\rSimulating FCFS..." << arrivalRate << "/" << arrivalRates[arrivalRates.size() - 1] << std::flush;
        s = simulatePriorityBased<FCFSPriorityComparator>(numProcesses, arrivalRate, serviceTime, 0,
                                                                       queryInterval, false, false);
        csvOut << arrivalRate << "," << s.avgTurnaroundTime << "," << s.throughput << "," << s.avgCpuUtil
               << "," << s.avgReadyQueueSize << endl;
    }
    csvOut.close();
    cout << "\rSimulating FCFS...done" << endl;

    // SRTF
    csvOut.open("SRTF.csv");
    for (double arrivalRate : arrivalRates) {
        cout << "\rSimulating SRTF..." << arrivalRate << "/" << arrivalRates[arrivalRates.size() - 1] << std::flush;
        s = simulatePriorityBased<SRTFPriorityComparator>(numProcesses, arrivalRate, serviceTime, 0,
                                                          queryInterval, true, false);
        csvOut << arrivalRate << "," << s.avgTurnaroundTime << "," << s.throughput << "," << s.avgCpuUtil
               << "," << s.avgReadyQueueSize << endl;
    }
    csvOut.close();
    cout << "\rSimulating SRTF...done" << endl;

    // HRRN
    csvOut.open("HRRN.csv");
    for (double arrivalRate : arrivalRates) {
        cout << "\rSimulating HRRN..." << arrivalRate << "/" << arrivalRates[arrivalRates.size() - 1] << std::flush;
        s = simulatePriorityBased<HRRNPriorityComparator>(numProcesses, arrivalRate, serviceTime, 0,
                                                          queryInterval, false, true);
        csvOut << arrivalRate << "," << s.avgTurnaroundTime << "," << s.throughput << "," << s.avgCpuUtil
               << "," << s.avgReadyQueueSize << endl;
    }
    csvOut.close();
    cout << "\rSimulating HRRN...done" << endl;
*/

    // RR
    double quantums[] {0.01, 0.2};
    for (auto quantum : quantums) {
        csvOut.open("RR(" + to_string(quantum) + ").csv");
        for (double arrivalRate : arrivalRates) {
            cout << "\rSimulating RR(" << quantum << ")..." << arrivalRate << "/" << arrivalRates[arrivalRates.size() - 1] << std::flush;
            s = simulatePriorityBased<FCFSPriorityComparator>(numProcesses, arrivalRate, serviceTime, quantum,
                                                              queryInterval, false, false);
            csvOut << arrivalRate << "," << s.avgTurnaroundTime << "," << s.throughput << "," << s.avgCpuUtil
                   << "," << s.avgReadyQueueSize << endl;
        }
        csvOut.close();
        cout << "\rSimulating RR" << quantum << "...done" << endl;
    }

    cout << "Finished all simulations." << endl;
}

int main(int argc, char* argv[]) {
//    srand((uint) time(nullptr));
    srand(2);

    int numProcesses = 10000;
    double queryInterval = 0.01;
    int scheduler, arrivalRate;
    double serviceTime, quantumLength;

    if (argc >= 2 && stoi(argv[1]) == -1) {
        runAllSimulations();
        return 0;
    } else if (argc == 4 + 1) {

        scheduler = stoi(argv[1]);
        arrivalRate = stoi(argv[2]);
        serviceTime = stod(argv[3]);
        quantumLength = stod(argv[4]);

        Statistics s {};

        switch (scheduler) {
            case 1:
                s = simulatePriorityBased<FCFSPriorityComparator>(numProcesses, arrivalRate, serviceTime, 0,
                                                                  queryInterval, false, false);
                break;
            case 2:
                s = simulatePriorityBased<SRTFPriorityComparator>(numProcesses, arrivalRate, serviceTime, 0,
                                                                  queryInterval, true, false);
                break;
            case 3:
                s = simulatePriorityBased<HRRNPriorityComparator>(numProcesses, arrivalRate, serviceTime, 0,
                                                                  queryInterval, false, true);
                break;
            case 4:
                s = simulatePriorityBased<FCFSPriorityComparator>(numProcesses, arrivalRate, serviceTime, quantumLength,
                                                                  queryInterval, false, false);
                break;
            default:
                break;
        }

        s.display();

        return 0;

    } else return 1;

}