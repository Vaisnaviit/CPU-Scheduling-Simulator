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

# Sample Input Format

Enter number of processes: 3
Enter name, arrival time, and service time for process 1: P1 0 5
Enter name, arrival time, and service time for process 2: P2 2 3
Enter name, arrival time, and service time for process 3: P3 4 1
Choose Algorithm:
1. FCFS
2. RR
3. SPN
4. SRT
5. HRRN
6. AGING
Enter choice: 2
Enter quantum for Round Robin: 2
Choose Output Format:
1. Trace
2. Statistics
Enter choice: 1
# Example Outputs
- Trace View

0 1 2 3 4 5 6 7 8 9 ...
------------------------------------------------
P1     |*|*|.|*|*|.|.|.|.|.|
P2     |.|.|*|*|.|*|.|.|.|.|
P3     |.|.|.|.|.|.|*|.|.|.|
------------------------------------------------

- Statistics View

RR-2
Process    |  P1  |  P2  |  P3  |
Arrival    |  0   |  2   |  4   |
Service    |  5   |  3   |  1   | Mean
Finish     | 10   | 9    | 7    |-----|
Turnaround | 10  | 7    | 3    |  6.67|
NormTurn   | 2.00| 2.33 | 3.00 |  2.44|

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
