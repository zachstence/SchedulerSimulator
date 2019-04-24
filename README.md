# SchedulerSimulator
A C++ program that simulates various CPU scheduling algorithms. This project was done as part of my CS 4328 
Operating Systems class at Texas State University. I have included the assignment sheet on the repository, and plan on
adding more features to my code in the future as a fun exercise.

## Downloading
Simply clone my GitHub repository and you have all the code ready to go. No extra requirements to install.

`$ git clone https://github.com/zachstence/SchedulerSimulator`

## Running Simulations
First, compile the main class

`$ g++ main.cpp`

#### One scheduler at a time
To simulate one scheduling algorithm with a specified arrival rate, service time and quantum length (only used in RR),
run the following command, choosing from the following algorithms
1. First Come First Serve (FCFS)
2. Shortest Remaining Time First (SRTF)
3. Highest Response Ratio Next (HRRN)
4. Round Robin (RR) with specified quantum length

`$ ./a.out <scheduler> <arrival rate> <service time> <quantum length>`

For example:
```bash
$ ./a.out 2 10 0.06 0.01
 Avg. Turnaround Time :  0.094
           Throughput :  9.933
        Avg. CPU Util :  0.593
  Avg. in Ready Queue :  0.344
```

#### All schedulers with results saved
To simulate all the scheduling algorithms with arrival rates from 1-30 processes/second, a service time of 0.06 seconds,
and quantum lengths of 0.01 and 0.2 seconds (for round robin), simply provide '-1' as the scheduler choice
```bash
$ ./a.out -1
Simulating FCFS...done
Simulating SRTF...done
Simulating HRRN...done
Simulating RR(0.01)...done
Simulating RR(0.2)...done
Finished all simulations.
```
The results will be saved in 5 CSV files, each named for its corresponding scheduler. The CSV files have 4 columns
(arrival rate, average turnaround time, throughput, and average processes in the ready queue), each holding values 
for a different iteration. These files can then easily be imported into a spreadsheet application to make 
[graphs](results/graphs/) from the CSV [results](results/) like I have included on this repo.

## Report
I have written a [short report](results/report.pdf) detailing my observations based on the output of my code and the graphs I 
have generated.

------------------------------
Zach Stence, 3/29/2019
