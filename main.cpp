#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <cmath>

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

struct Process {
    int id;
    double arrivalTime;
    double completionTime;
    double serviceTime;
    double serviceTimeLeft;
};

enum EventType {
    ARRIVAL, DEPARTURE, TIMEOUT, QUERY
};

struct Event {
    double time;
    Process* process;
    EventType type;

    bool operator<(const Event& other) const {
        return this->time > other.time;
    }
};

struct Statistics {
    double avgTurnaroundTime;
    double throughput;
    double avgCpuUtil;
    double avgReadyQueueSize;
};

double inversePoisson(double rate) {
    double y = (double) rand() / RAND_MAX;
    return -1 * log(1 - y) / rate;
}

void scheduleEvent(priority_queue<Event> &eventQueue, double time, Process* p, EventType type) {
    eventQueue.push({time, p, type});
}

void printStatistics(Statistics s) {
    printf("Avg. Turnaround Time : %6.3f\n"
           "          Throughput : %6.3f\n"
           "       Avg. CPU Util : %6.3f\n"
           " Avg. in Ready Queue : %6.3f\n",
           s.avgTurnaroundTime, s.throughput, s.avgCpuUtil, s.avgReadyQueueSize
    );
}

Statistics simulateFCFS(int numProcesses, double arrivalRate, double serviceTime, double queryInterval) {
    priority_queue<Event> eventQueue;
    queue<Process*> readyQueue;
    vector<Process*> processes;

    double clock = 0;
    bool cpuIdle = true;

    // Schedule first process arrival
    double t = inversePoisson(1 / serviceTime);
    auto firstProcess = new Process {0, clock, -1, t, t};
    processes.push_back(firstProcess);
    scheduleEvent(eventQueue, clock, firstProcess, ARRIVAL);

    // Schedule first query event
    scheduleEvent(eventQueue, clock, {}, QUERY);

    int processesSimulated = 0;
    double cpuIdleTime = 0;
    double lastCpuBusyTime = 0;
    int totalInReadyQueue = 0;

    while (processesSimulated < numProcesses && !eventQueue.empty()) {

        Event current = eventQueue.top();
        eventQueue.pop();
        clock = current.time;

        // If arrival event
        if (current.type == ARRIVAL) {
            // If CPU is idle, let arriving event use CPU and schedule its departure
            if (cpuIdle) {
                cpuIdle = false;
                cpuIdleTime += clock - lastCpuBusyTime;
                scheduleEvent(eventQueue, clock + current.process->serviceTime, current.process, DEPARTURE);
            }
            // If CPU is not idle, add arriving event to ready queue
            else {
                readyQueue.push(current.process);
            }

            // Schedule next event's arrival
            int nextId = current.process->id + 1;
            double nextTime = clock + inversePoisson(arrivalRate);
            double service = inversePoisson(1 / serviceTime);
            auto nextArrival = new Process {nextId, nextTime, -1, service, service};
            processes.push_back(nextArrival);
            scheduleEvent(eventQueue, nextTime, nextArrival, ARRIVAL);
        }

        // If departure event
        else if (current.type == DEPARTURE) {
            // Increment number of processes simulated at each departure
            processesSimulated++;
            current.process->completionTime = clock;

            // If ready queue is empty, set CPU to idle
            if (readyQueue.empty()) {
                cpuIdle = true;
                lastCpuBusyTime = clock;
            }
            // If ready queue is not empty, get next process from ready queue and let it use CPU and schedule its
            // departure
            else {
                Process* p = readyQueue.front();
                cpuIdle = false;
                readyQueue.pop();
                scheduleEvent(eventQueue, clock + p->serviceTime, p, DEPARTURE);
            }
        }

        else if (current.type == QUERY) {
            totalInReadyQueue += readyQueue.size();
            scheduleEvent(eventQueue, clock + queryInterval, {}, QUERY);
        }

    }

    // Sum turnaround times for statistics
    double totalTurnaroundTime = 0;
    for (auto process : processes) {
        if (process->completionTime == -1) continue;
        double turnaround = process->completionTime - process->arrivalTime;
        totalTurnaroundTime += turnaround;
    }

    double avgTurnaroundTime = totalTurnaroundTime / numProcesses;
    double throughput = processesSimulated / clock;
    double avgCpuUtil = 1 - (cpuIdleTime / clock);
    double avgReadyQueueSize = (double) totalInReadyQueue / (clock / queryInterval);

    return {avgTurnaroundTime, throughput, avgCpuUtil, avgReadyQueueSize};
}

class SRTFComparator {
    bool operator()(Process* p1, Process* p2) {
        return p1->serviceTimeLeft < p2->serviceTimeLeft;
    }
};

Statistics simulateSRTF(int numProcesses, double arrivalRate, double serviceTime, double queryInterval) {
    priority_queue<Event> eventQueue;
    priority_queue<Process*, vector<Process*>, SRTFComparator> readyQueue;
    vector<Process*> processes;

    Process p0 {0, 0, -1, 5, 5};
    Process p1 {1, 3, -1, 6, 6};
    Process p2 {2, 2, -1, 3, 4};
    processes.push_back(&p0);
    processes.push_back(&p1);
    processes.push_back(&p2);

    for (auto p : processes) {
        cout << p->id << " " << p->serviceTimeLeft << endl;
    }

/*    double clock = 0;
    bool cpuIdle = true;

    // Schedule first process arrival
    double t = inversePoisson(1 / serviceTime);
    auto firstProcess = new Process {0, clock, -1, t, t};
    processes.push_back(firstProcess);
    scheduleEvent(eventQueue, clock, firstProcess, ARRIVAL);

    // Schedule first query event
    scheduleEvent(eventQueue, clock, {}, QUERY);

    int processesSimulated = 0;
    double cpuIdleTime = 0;
    double lastCpuBusyTime = 0;
    int totalInReadyQueue = 0;

    while (processesSimulated < numProcesses && !eventQueue.empty()) {

        Event current = eventQueue.top();
        eventQueue.pop();
        clock = current.time;

        // If arrival event
        if (current.type == ARRIVAL) {
            // If CPU is idle, let arriving event use CPU
            if (cpuIdle) {
                cpuIdle = false;
                cpuIdleTime += clock - lastCpuBusyTime;
            }
            // If CPU is not idle, add arriving event to ready queue
            else {

            }
        }
    }*/
}

Statistics simulateHRRN(int numProcesses, double arrivalRate, double serviceTime, double queryInterval) {

}

Statistics simulateRR(int numProcesses, double arrivalRate, double serviceTime, double quantumLength, double queryInterval) {

}


// TODO run all simulations and save output in CSV to make graphs in spreadsheet
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

        printStatistics(s);

        return 0;

    } else return 1;

}