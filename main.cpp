#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>

#include "EventQueue.h"

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

Statistics simulateFCFS(int numProcesses, double arrivalRate, double serviceTime, double queryInterval) {
    EventQueue eventQueue;
    queue<Process*> readyQueue;
    vector<Process*> processes;

    double clock = 0;
    bool cpuIdle = true;

    // Schedule first process arrival
    auto firstProcess = new Process(0, 0, inversePoisson(1 / serviceTime));
    processes.push_back(firstProcess);
    eventQueue.scheduleEvent(clock, firstProcess, ARRIVAL);

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
            // If CPU is idle, let arriving event use CPU and schedule its departure
            if (cpuIdle) {
                cpuIdle = false;
                cpuIdleTime += clock - lastCpuBusyTime;
                eventQueue.scheduleEvent(clock + current.process->getServiceTime(), current.process, DEPARTURE);
            }
                // If CPU is not idle, add arriving event to ready queue
            else {
                readyQueue.push(current.process);
            }

            // Schedule next event's arrival
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
                lastCpuBusyTime = clock;
            }
            // If ready queue is not empty, get next process from ready queue and let it use CPU and schedule its
            // departure
            else {
                Process* p = readyQueue.front();
                readyQueue.pop();
                cpuIdle = false;
                eventQueue.scheduleEvent(clock + p->getServiceTime(), p, DEPARTURE);
            }
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

class SRTFComparator {
public:
    bool operator()(Process* p1, Process* p2) {
        return p1->getServiceTimeLeft() < p2->getServiceTimeLeft();
    }
};

Statistics simulateSRTF(int numProcesses, double arrivalRate, double serviceTime, double queryInterval) {
    EventQueue eventQueue;
    priority_queue<Process*, vector<Process*>, SRTFComparator> readyQueue;
    vector<Process*> processes;
    Process* onCpu;

    double clock = 0;
    bool cpuIdle = true;

    // Schedule first process arrival
    auto firstProcess = new Process(0, 0, inversePoisson(1 / serviceTime));
    processes.push_back(firstProcess);
    eventQueue.scheduleEvent(clock, firstProcess, ARRIVAL);

    // Schedule first query event
    eventQueue.scheduleEvent(clock, nullptr, QUERY);

    int processesSimulated = 0;
    double cpuIdleTime = 0;
    double lastCpuBusyTime = 0;
    int totalInReadyQueue = 0;

    while (processesSimulated < numProcesses && !eventQueue.empty()) {

        Event current = eventQueue.getEvent();
        clock = current.time;

        // If arrival event
        if (current.type == ARRIVAL) {
            // If CPU is idle, let arriving event use CPU and schedule tentative departure
            if (cpuIdle) {
                cpuIdle = false;
                current.process->setLastTimeOnCpu(clock);
                cpuIdleTime += clock - lastCpuBusyTime;
                eventQueue.scheduleEvent(clock + current.process->getServiceTimeLeft(), current.process, DEPARTURE);
                onCpu = current.process;
            }
            // If CPU is busy, check for preemption
            // Preemption process:
            //   - Update onCpu remaining time
            //   - Delete tentative departure of onCpu
            //   - Put onCpu in ready queue
            //   - Put front of ready queue on CPU
            //   - Schedule new tentative departure
            else if (current.process->getServiceTimeLeft() < onCpu->getServiceTimeLeft()) {
                onCpu->decreaseServiceTimeLeft(clock - onCpu->getLastTimeOnCpu());
                bool success = eventQueue.unscheduleDeparture(onCpu->getId());
                if (!success) printf("Scheduling error! Couldn't remove tentative departure!"); // TODO remove later?
                readyQueue.push(onCpu);
                onCpu = readyQueue.top();
                readyQueue.pop();
                eventQueue.scheduleEvent(clock + onCpu->getServiceTimeLeft(), onCpu, DEPARTURE);
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
                lastCpuBusyTime = clock;
            }
            // If ready queue is not empty, put next process from ready queue on CPU
            // and schedule its tentative departure
            else {
                Process* p = readyQueue.top();
                readyQueue.pop();
                cpuIdle = false;
                eventQueue.scheduleEvent(clock + p->getServiceTimeLeft(), p, DEPARTURE);
            }
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

Statistics simulateHRRN(int numProcesses, double arrivalRate, double serviceTime, double queryInterval) {

}

Statistics simulateRR(int numProcesses, double arrivalRate, double serviceTime, double quantumLength, double queryInterval) {

}

void runAllSimulations() {

    double arrivalRates[] {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};

    int numProcesses = 10000;
    double serviceTime = 0.06;
    double queryInterval = 0.1;

    Statistics s {};

    // FCFS
    ofstream csvOut;
    csvOut.open("FCFS.csv");
    for (double arrivalRate : arrivalRates) {
        s = simulateFCFS(numProcesses, arrivalRate, serviceTime, queryInterval);
        csvOut << arrivalRate << "," << s.avgTurnaroundTime << "," << s.throughput << "," << s.avgCpuUtil
               << "," << s.avgReadyQueueSize << endl;
    }
    csvOut.close();

    // SRTF
    csvOut.open("SRTF.csv");
    for (double arrivalRate : arrivalRates) {
        s = simulateSRTF(numProcesses, arrivalRate, serviceTime, queryInterval);
        csvOut << arrivalRate << "," << s.avgTurnaroundTime << "," << s.throughput << "," << s.avgCpuUtil
               << "," << s.avgReadyQueueSize << endl;
    }
    csvOut.close();

    // HRRN
    csvOut.open("HRRN.csv");
    for (double arrivalRate : arrivalRates) {
        s = simulateHRRN(numProcesses, arrivalRate, serviceTime, queryInterval);
        csvOut << arrivalRate << "," << s.avgTurnaroundTime << "," << s.throughput << "," << s.avgCpuUtil
               << "," << s.avgReadyQueueSize << endl;
    }
    csvOut.close();

    // RR
    double quantums[] {0.01, 0.2};
    for (auto quantum : quantums) {
        csvOut.open("RR.csv");
        for (double arrivalRate : arrivalRates) {
            s = simulateRR(numProcesses, arrivalRate, serviceTime, quantum, queryInterval);
            csvOut << arrivalRate << "," << s.avgTurnaroundTime << "," << s.throughput << "," << s.avgCpuUtil
                   << "," << s.avgReadyQueueSize << endl;
        }
        csvOut.close();
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

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
                s = simulateFCFS(numProcesses, arrivalRate, serviceTime, queryInterval);
                break;
            case 2:
                s = simulateSRTF(numProcesses, arrivalRate, serviceTime, queryInterval);
                break;
            case 3:
                s = simulateHRRN(numProcesses, arrivalRate, serviceTime, queryInterval);
                break;
            case 4:
                s = simulateRR(numProcesses, arrivalRate, serviceTime, quantumLength, queryInterval);
                break;
            default:
                break;
        }

        s.display();

        return 0;

    } else return 1;

}