/**
 * @author zachstence / zms22
 * @since 3/24/2019
 */

#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <numeric>

#include "EventQueue.h"
#include "ReadyQueue.h"
#include "PriorityComparator.h"

using std::cerr;
using std::cout;
using std::endl;
using std::stoi;
using std::stod;
using std::to_string;
using std::ofstream;
using std::random_device;
using std::mt19937;
using std::uniform_real_distribution;
using std::vector;
using std::iota;

/**
 * Simple structure to hold statistics about a scheduling algorithms performance.
 */
struct Statistics {
    double avgTurnaroundTime;
    double throughput;
    double avgCpuUtil;
    double avgReadyQueueSize;

    /**
     * Prints a string representation of the statistics.
     */
    void display() {
        printf("Avg. Turnaround Time : %6.3f\n"
               "          Throughput : %6.3f\n"
               "       Avg. CPU Util : %6.3f\n"
               " Avg. in Ready Queue : %6.3f\n",
               this->avgTurnaroundTime, this->throughput, this->avgCpuUtil, this->avgReadyQueueSize
        );
    }
};

/**
 * Uses the inverse of the probability distribution function for a poisson distribution to translate a random
 * number [0,1] following a uniform distribution into a random number [0,1] following a poisson distribution.
 * In the context of schedulers, it generates inter-arrival times or service times following an average rate given.
 * @param rate The average rate of arrival or service.
 * @return A random number following a poisson distribution given the rate.
 */
double inversePoisson(double rate) {
    // Setup random number generator
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0, 1);

    // Generate random number and find corresponding number according to poisson distribution
    double y = dis(gen);
    return -1 * log(1 - y) / rate;
}

/**
 * Simulates a priority based scheduling algorithm with various options.
 * @tparam PriorityComparator The comparator to use for determining process priority. Must implement operator() for
 *         determining process priorities.
 * @param numProcesses The number of processes to simulate.
 * @param arrivalRate The average arrival rate (in processes/second) of the processes.
 * @param serviceTime The average service time (in seconds) of the processes.
 * @param quantumLength The quantum length (in seconds) to use in a round robin simulation (<0 to not do round robin)
 * @param queryInterval The interval at which various statistics are updated. Lower numbers improve statistics accuracy
 *        but increase running time.
 * @param doPreemption Whether or not the simulation will do preemption.
 * @param dynamicPriority Whether or not the simulation will use a dynamic priority scheme.
 * @return A Statistics struct containing the average turnaround time of all processes simulated (in seconds),
 *         the throughput of the simulation (in processes/second), the average CPU utilization, and the average ready queue
 *         size.
 */
template <class PriorityComparator>
Statistics simulatePriorityBased(int numProcesses, double arrivalRate, double serviceTime, double quantumLength,
                                 double queryInterval, bool doPreemption, bool dynamicPriority) {
    // Create data structures to hold events and processes
    EventQueue eventQueue;
    ReadyQueue<PriorityComparator> readyQueue;
    vector<Process*> processes;

    // Pointer to the process currently using the CPU
    Process* onCpu = nullptr;

    double clock = 0;
    bool cpuIdle = true;

    // Schedule first process arrival
    auto firstProcess = new Process(0, 0, inversePoisson(1 / serviceTime));
    processes.push_back(firstProcess);
    eventQueue.scheduleEvent(clock, firstProcess, ARRIVAL);

    // Schedule first timeout event for round robin (if quantumLength isn't negative or 0)
    if (quantumLength > 0)
        eventQueue.scheduleEvent(clock + quantumLength, nullptr, TIMEOUT);

    // Schedule first query event
    eventQueue.scheduleEvent(clock + queryInterval, nullptr, QUERY);

    // Variables to hold statistics about simulation
    int processesSimulated = 0;
    double cpuIdleTime = 0;
    double lastCpuBusyTime = 0;
    int totalInReadyQueue = 0;

    // Loop while need to simulate more processes
    while (processesSimulated < numProcesses && !eventQueue.empty()) {

        // Get next event and update clock
        Event current = eventQueue.getEvent();
        clock = current.getTime();

        // If event is an arrival
        if (current.getType() == ARRIVAL) {
            // If CPU is idle
            if (cpuIdle) {
                // Set CPU to busy and update idle time
                cpuIdle = false;
                cpuIdleTime += clock - lastCpuBusyTime;
                // Assign arriving process to CPU
                current.getProcess()->setLastTimeAssignedCpu(clock);
                onCpu = current.getProcess();
                // Schedule arriving process' tentative departure (may be unscheduled later due to preemption or timeout)
                eventQueue.scheduleEvent(clock + current.getProcess()->getServiceTimeLeft(), current.getProcess(), DEPARTURE);
            }
            // If CPU is busy
            else {
                // Add arriving process to the ready queue
                readyQueue.add(current.getProcess());
                // If dynamic priority scheme, update statistics and resort ready queue
                if (dynamicPriority) {
                    readyQueue.updateWaitTimes(clock);
                    readyQueue.sort();
                }

                // If doing preemption, update onCpu's statistics
                if (doPreemption) {
                    onCpu->setServiceTimeLeft(onCpu->getServiceTimeLeft() - (clock - onCpu->getLastTimeAssignedCpu()));

                    Process* candidate = readyQueue.getFront();
                    // Compare process currently on CPU with highest priority from the ready queue,
                    // if front of ready queue has a higher priority, we switch to that process
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
                    // If front of ready queue is not higher priority
                    else {
                        // Place it back in the ready queue
                        readyQueue.add(candidate);
                    }
                    // Whichever process is now using the CPU, update the last time it was assigned to now
                    onCpu->setLastTimeAssignedCpu(clock);
                }
            }

            // Schedule next process arrival
            int nextId = current.getProcess()->getId() + 1;
            double nextArrivalTime = clock + inversePoisson(arrivalRate);
            auto nextArrival = new Process(nextId, nextArrivalTime, inversePoisson(1 / serviceTime));
            processes.push_back(nextArrival);
            eventQueue.scheduleEvent(nextArrivalTime, nextArrival, ARRIVAL);
        }
        // If event is a departure
        else if (current.getType() == DEPARTURE) {
            // Increment number of processes simulated at each departure
            processesSimulated++;
            // Update completion time of departing process
            current.getProcess()->setCompletionTime(clock);

            // If ready queue is empty, set CPU to idle and update statistics
            if (readyQueue.empty()) {
                cpuIdle = true;
                onCpu = nullptr;
                lastCpuBusyTime = clock;
            }
            // If ready queue is not empty, put next process from ready queue on CPU and schedule its tentative departure
            else {
                // If dynamic priority scheme, update statistics and resort ready queue
                if (dynamicPriority) {
                    readyQueue.updateWaitTimes(clock);
                    readyQueue.sort();
                }

                // Assign front of ready queue to CPU
                Process* p = readyQueue.getFront();
                p->setLastTimeAssignedCpu(clock);
                onCpu = p;
                eventQueue.scheduleEvent(clock + p->getServiceTimeLeft(), p, DEPARTURE);
            }
        }
        // If event is a timeout (round robin only)
        else if (current.getType() == TIMEOUT) {
            // If the CPU is idle, do nothing
            if (cpuIdle);
            // If CPU is busy, switch to process at the front of the ready queue
            else {
                // Update current process' remaining service time, unschedule tentative departure, and place in ready queue
                onCpu->setServiceTimeLeft(onCpu->getServiceTimeLeft() - (clock - onCpu->getLastTimeAssignedCpu()));
                eventQueue.unscheduleDeparture(onCpu->getId());
                readyQueue.add(onCpu);

                // If dynamic priority scheme, update statistics and resort ready queue
                if (dynamicPriority) {
                    readyQueue.updateWaitTimes(clock);
                    readyQueue.sort();
                }

                // Assign front of ready queue to CPU, schedule tentative departure, and update last time assigned CPU
                onCpu = readyQueue.getFront();
                eventQueue.scheduleEvent(clock + onCpu->getServiceTimeLeft(), onCpu, DEPARTURE);
                onCpu->setLastTimeAssignedCpu(clock);
            }
            // Schedule next timeout event
            eventQueue.scheduleEvent(clock + quantumLength, nullptr, TIMEOUT);
        }
        // If event is a query (updating statistics)
        else if (current.getType() == QUERY) {
            // Update statistics
            totalInReadyQueue += readyQueue.size();
            // Schedule next query event
            eventQueue.scheduleEvent(clock + queryInterval, nullptr, QUERY);
        }
    }

    // Sum turnaround times for calculating average turnaround time
    double totalTurnaroundTime = 0;
    for (auto process : processes) {
        if (process->getCompletionTime() == -1) continue;
        double turnaround = process->getCompletionTime() - process->getArrivalTime();
        totalTurnaroundTime += turnaround;
    }

    // Calculate statistics
    double avgTurnaroundTime = totalTurnaroundTime / numProcesses;
    double throughput = processesSimulated / clock;
    double avgCpuUtil = 1 - (cpuIdleTime / clock);
    double avgReadyQueueSize = (double) totalInReadyQueue / (clock / queryInterval);

    // return statistics
    return {avgTurnaroundTime, throughput, avgCpuUtil, avgReadyQueueSize};
}

/**
 * Runs all the simulations specified in the assignment for different arrival rates and saves their statistics
 * in a CSV file for easy importing into a spreadsheet for graph creation.
 */
void runAllSimulations() {

    // Vector of average arrival rates and average service time run simulations with
    vector<double> arrivalRates (30);
    iota(arrivalRates.begin(), arrivalRates.end(), 1);
    double serviceTime = 0.06;

    // Number of processes to simulate (higher = more accurate statistics)
    // and query interval (lower = more accurate statistics)
    int numProcesses = 10000;
    double queryInterval = 0.01;

    Statistics s {};
    ofstream csvOut;

    // Simulate various algorithms and save their statistics for each arrival rate in CSV format
    // First Come First Serve (FCFS)
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

    // Shortest Remaining Time First (SRTF)
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

    // Highest Response Ratio Next (HRRN)
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

    // Round Robin (RR)
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
        cout << "\rSimulating RR(" << quantum << ")...done" << endl;
    }

    cout << "Finished all simulations." << endl;
}

/**
 * Parses command line input and runs a scheduling simulations.
 * To run a simulation, specify the scheduler, arrival rate, service time, and quantum length:
 *   <sched> <arrival_rate> <service_time> <quantum_length>
 * The quantum length will be ignored unless the round robin scheduler is being simulated.
 * Available schedulers are:
 *   (-1) Run all algorithms and save results to CSV
 *    (1) First Come First Serve (FCFS)
 *    (2) Shortest Remaining Time First (SRTF)
 *    (3) Highest Response Ratio Next (HRRN)
 *    (4) Round Robin (RR)
 */
int main(int argc, char* argv[]) {

    // Number of processes to simulate (higher = more accurate statistics)
    // and query interval (lower = more accurate statistics)
    int numProcesses = 10000;
    double queryInterval = 0.01;

    // Variables for input parameters
    int scheduler, arrivalRate;
    double serviceTime, quantumLength;

    // Parse command line input
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
                std::cerr << "Invalid scheduler choice. Available choices:\n"
                             "(-1) Run all algorithms and save results to CSV\n"
                             " (1) First Come First Serve (FCFS)\n"
                             " (2) Shortest Remaining Time First (SRTF)\n"
                             " (3) Highest Response Ratio Next\n"
                             " (4) Round Robin (RR)" << endl;
                return 1;
        }

        s.display();

        return 0;

    } else {
        std::cerr << "Invalid number of arguments. Usage: \"<sched> <arrival_rate> <service_time> <quantum_length>\".\n"
                     "Use sched=-1 to run all algorithms with varying arrival rates and save results to CSV." << endl;
    }

}