# CPU Scheduling Simulator
This is a CPU Scheduling Simulator implemented in C++. It provides an interactive console-based interface for visualizing and analyzing multiple CPU scheduling algorithms. Designed for educational purposes, this simulator helps students and developers understand how various scheduling techniques work in real-time with trace timelines and statistical metrics.

# Features
Supports the following scheduling algorithms:
- First Come First Serve (FCFS)
- Round Robin (RR) – with user-defined quantum
- Shortest Process Next (SPN)
- Shortest Remaining Time (SRT)
- Highest Response Ratio Next (HRRN)
- Aging Algorithm (priority-based aging)

Displays two types of output:
- Trace View: Timeline showing execution (*) and waiting (.) per process
- Statistics View: Shows arrival, service, finish, turnaround, and normalized turnaround time for each process

# File Structure
main.cpp: Main source file containing all algorithm implementations, utilities, and I/O handling

# How It Works
- Input:
User is prompted to enter the number of processes.
For each process: name, arrival time, and service time.
User selects the scheduling algorithm.
If Round Robin is selected, the time quantum is also requested.
The user then selects the output format: Trace or Statistics.

- Processing:
The selected algorithm is run.
Internally, the code maintains a timeline matrix to visualize execution.
Metrics like finish time, turnaround time, and normalized turnaround time are calculated.

- Output:
Trace View: Shows per-time execution trace for each process.
Stats View: Outputs summarized performance statistics including averages.

# Compilation & Execution
To compile and run:
bash
Copy
Edit
g++ main.cpp -o scheduler
./scheduler
Make sure you are using a C++11 compatible compiler or above.

# Educational Use
This simulator is ideal for:
Operating systems coursework
Visual demonstrations of scheduling algorithms
Comparative performance analysis

# Notes
Timeline length auto-adjusts to fit all processes.
Round Robin algorithm dynamically enqueues arriving processes during execution.
Waiting periods are marked with . and execution with *.
