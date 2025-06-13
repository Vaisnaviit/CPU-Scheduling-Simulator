#include <bits/stdc++.h>
using namespace std;

// Global Constants for Operation Types
const string TRACE = "trace";
const string SHOW_STATISTICS = "stats";

// Global variables for process scheduling data
string operation;
int last_instant = 0, process_count = 0;
vector<tuple<string, int, int>> processes; // Stores process name, arrival time, service time
vector<vector<char>> timeline; // Represents the timeline of process execution
unordered_map<string, int> processToIndex; // Maps process names to their index in the processes vector

// Results vectors
vector<int> finishTime;     // Stores the finish time for each process
vector<int> turnAroundTime; // Stores the turnaround time for each process
vector<float> normTurn;     // Stores the normalized turnaround time for each process

// Array of algorithm names for printing statistics
const string ALGORITHMS[9] = {"", "FCFS", "RR-", "SPN", "SRT", "HRRN", "FB-1", "FB-2i", "AGING"};

// Function to clear the timeline for a new simulation
void clear_timeline() {
    for (int i = 0; i < last_instant; i++)
        for (int j = 0; j < process_count; j++)
            timeline[i][j] = ' ';
}

// Helper functions to get process attributes from a tuple
int getArrivalTime(tuple<string, int, int>& a) { return get<1>(a); }
int getServiceTime(tuple<string, int, int>& a) { return get<2>(a); }
string getProcessName(tuple<string, int, int>& a) { return get<0>(a); }

// Function to calculate the response ratio for HRRN
double calculate_response_ratio(int wait_time, int service_time) {
    return (wait_time + service_time) * 1.0 / service_time;
}

// Fills in '.' for wait times in the timeline after a process finishes
void fillInWaitTime() {
    for (int i = 0; i < process_count; i++) {
        int arrivalTime = getArrivalTime(processes[i]);
        for (int k = arrivalTime; k < finishTime[i]; k++) {
            if (timeline[k][i] != '*') timeline[k][i] = '.'; // Mark as waiting if not running
        }
    }
}

// First Come First Serve (FCFS) scheduling algorithm
void firstComeFirstServe() {
    // Start time is the arrival time of the first process
    int time = getArrivalTime(processes[0]);
    for (int i = 0; i < process_count; i++) {
        int arrival = getArrivalTime(processes[i]);
        int service = getServiceTime(processes[i]);

        // Calculate finish time, turnaround time, and normalized turnaround time
        finishTime[i] = time + service;
        turnAroundTime[i] = finishTime[i] - arrival;
        normTurn[i] = turnAroundTime[i] * 1.0 / service;

        // Fill the timeline: '*' for running, '.' for waiting
        for (int j = time; j < finishTime[i]; j++) timeline[j][i] = '*';
        for (int j = arrival; j < time; j++) timeline[j][i] = '.';
        time = finishTime[i]; // Update current time
    }
}

// Round Robin (RR) scheduling algorithm
void roundRobin(int quantum) {
    queue<int> q; // Queue to hold indices of ready processes
    vector<int> remaining(process_count); // Remaining service time for each process

    // Initialize remaining service times
    for (int i = 0; i < process_count; ++i) remaining[i] = getServiceTime(processes[i]);

    int time = 0, j = 0; // Current time and index for processes that have arrived

    // Loop until all processes are completed or time reaches last_instant
    while (time < last_instant || !q.empty()) {
        // Add newly arrived processes to the queue
        while (j < process_count && getArrivalTime(processes[j]) <= time) {
            q.push(j++);
        }

        if (!q.empty()) {
            int idx = q.front(); // Get the process at the front of the queue
            q.pop(); // Remove it from the queue

            // Determine how long the process will run in this quantum
            int run = min(quantum, remaining[idx]);

            // Mark the timeline for the running process
            for (int t = 0; t < run; ++t) {
                if (time + t < last_instant) // Ensure we don't go out of bounds
                    timeline[time + t][idx] = '*';
            }

            time += run; // Advance time
            remaining[idx] -= run; // Decrease remaining service time

            // Add any processes that arrived during this quantum to the queue
            while (j < process_count && getArrivalTime(processes[j]) <= time) {
                q.push(j++);
            }

            // If the process is not yet finished, add it back to the queue
            if (remaining[idx] > 0) {
                q.push(idx);
            } else {
                // If the process is finished, calculate its metrics
                finishTime[idx] = time;
                turnAroundTime[idx] = time - getArrivalTime(processes[idx]);
                normTurn[idx] = turnAroundTime[idx] * 1.0 / getServiceTime(processes[idx]);
            }
        } else {
            // If the queue is empty, increment time to find the next arriving process
            time++;
        }
    }
    fillInWaitTime(); // Mark wait times in the timeline
}

// Shortest Process Next (SPN) scheduling algorithm
void shortestProcessNext() {
    vector<bool> done(process_count, false); // Tracks if a process is completed
    int time = 0; // Current time

    // Loop until all processes are completed
    while (count(done.begin(), done.end(), true) < process_count) {
        int idx = -1; // Index of the selected process
        int minST = INT_MAX; // Minimum service time found

        // Find the process with the shortest service time among arrived and not done processes
        for (int i = 0; i < process_count; ++i) {
            if (!done[i] && getArrivalTime(processes[i]) <= time && getServiceTime(processes[i]) < minST) {
                idx = i;
                minST = getServiceTime(processes[i]);
            }
        }

        if (idx == -1) {
            time++; // No process is ready, advance time
            continue;
        }

        // Execute the selected process
        for (int t = time; t < time + minST; ++t) {
            if (t < last_instant) // Ensure we don't go out of bounds
                timeline[t][idx] = '*';
        }
        time += minST; // Advance time by service time

        // Calculate metrics for the finished process
        finishTime[idx] = time;
        turnAroundTime[idx] = time - getArrivalTime(processes[idx]);
        normTurn[idx] = turnAroundTime[idx] * 1.0 / minST;
        done[idx] = true; // Mark process as done
    }
    fillInWaitTime(); // Mark wait times in the timeline
}

// Shortest Remaining Time (SRT) scheduling algorithm
void shortestRemainingTime() {
    vector<int> remaining(process_count); // Remaining service time for each process
    for (int i = 0; i < process_count; i++) remaining[i] = getServiceTime(processes[i]);

    vector<bool> done(process_count, false); // Tracks if a process is completed
    int time = 0, completed = 0; // Current time and number of completed processes

    // Loop until all processes are completed
    while (completed < process_count) {
        int idx = -1; // Index of the selected process
        int minRem = INT_MAX; // Minimum remaining time found

        // Find the process with the shortest remaining time among arrived and not done processes
        for (int i = 0; i < process_count; ++i) {
            if (!done[i] && getArrivalTime(processes[i]) <= time && remaining[i] < minRem && remaining[i] > 0) {
                idx = i;
                minRem = remaining[i];
            }
        }

        if (idx == -1) {
            time++; // No process is ready, advance time
            continue;
        }

        // Execute the selected process for one unit of time
        if (time < last_instant) // Ensure we don't go out of bounds
            timeline[time][idx] = '*';
        remaining[idx]--;
        time++;

        // If the process is finished
        if (remaining[idx] == 0) {
            finishTime[idx] = time;
            int arrival = getArrivalTime(processes[idx]);
            int service = getServiceTime(processes[idx]);
            turnAroundTime[idx] = time - arrival;
            normTurn[idx] = turnAroundTime[idx] * 1.0 / service;
            done[idx] = true;
            completed++;
        }
    }
    fillInWaitTime(); // Mark wait times in the timeline
}

// Highest Response Ratio Next (HRRN) scheduling algorithm
void highestResponseRatioNext() {
    vector<bool> done(process_count, false); // Tracks if a process is completed
    int time = 0; // Current time

    // Loop until all processes are completed
    while (count(done.begin(), done.end(), true) < process_count) {
        double maxRatio = -1.0; // Maximum response ratio found
        int idx = -1; // Index of the selected process

        // Find the process with the highest response ratio among arrived and not done processes
        for (int i = 0; i < process_count; ++i) {
            if (!done[i] && getArrivalTime(processes[i]) <= time) {
                int wait = time - getArrivalTime(processes[i]);
                int service = getServiceTime(processes[i]);
                double ratio = calculate_response_ratio(wait, service);
                if (ratio > maxRatio) {
                    maxRatio = ratio;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            time++; // No process is ready, advance time
            continue;
        }

        // Execute the selected process
        int st = getServiceTime(processes[idx]);
        for (int t = time; t < time + st; ++t) {
            if (t < last_instant) // Ensure we don't go out of bounds
                timeline[t][idx] = '*';
        }
        time += st; // Advance time by service time

        // Calculate metrics for the finished process
        finishTime[idx] = time;
        turnAroundTime[idx] = time - getArrivalTime(processes[idx]);
        normTurn[idx] = turnAroundTime[idx] * 1.0 / st;
        done[idx] = true; // Mark process as done
    }
    fillInWaitTime(); // Mark wait times in the timeline
}

// Aging scheduling algorithm (simple priority aging)
void agingAlgorithm() {
    vector<int> remaining(process_count); // Remaining service time for each process
    for (int i = 0; i < process_count; i++) remaining[i] = getServiceTime(processes[i]);

    vector<int> priority(process_count, 0); // Priority for each process (increases with age)
    vector<bool> done(process_count, false); // Tracks if a process is completed
    int time = 0; // Current time

    // Loop until all processes are completed
    while (count(done.begin(), done.end(), true) < process_count) {
        int idx = -1; // Index of the selected process

        // Find the process with the highest priority among arrived and not done processes
        for (int i = 0; i < process_count; ++i) {
            if (!done[i] && getArrivalTime(processes[i]) <= time) {
                priority[i]++; // Increase priority for waiting processes
                if (idx == -1 || priority[i] > priority[idx]) {
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            time++; // No process is ready, advance time
            continue;
        }

        // Execute the selected process for one unit of time
        if (time < last_instant) // Ensure we don't go out of bounds
            timeline[time][idx] = '*';
        remaining[idx]--;
        time++;

        // If the process is finished
        if (remaining[idx] == 0) {
            finishTime[idx] = time;
            turnAroundTime[idx] = time - getArrivalTime(processes[idx]);
            normTurn[idx] = turnAroundTime[idx] * 1.0 / getServiceTime(processes[idx]);
            done[idx] = true;
        }
    }
    fillInWaitTime(); // Mark wait times in the timeline
}

// Tracing Function: printTimeline
// This function prints the timeline of process execution, showing which process is running ('*') or waiting ('.') at each time instant.
void printTimelineOutput()
{
    // Prints the time instants (0-9 repeating)
    for (int i = 0; i <= last_instant; i++)
        cout << i % 10 << " ";
    cout << "\n";
    cout << "------------------------------------------------\n";

    // Iterates through each process
    for (int i = 0; i < process_count; i++)
    {
        // Prints the process name
        cout << getProcessName(processes[i]) << "     |";
        // Prints the state of the process at each time instant
        for (int j = 0; j < last_instant; j++)
        {
            cout << timeline[j][i] << "|";
        }
        cout << " \n";
    }
    cout << "------------------------------------------------\n";
}


// Statistics Functions

// printAlgorithm: Prints the name of the current scheduling algorithm.
void printAlgorithm(int algorithm_id_val, int quantum_val)
{
    if(algorithm_id_val == 2) // For Round Robin (RR-) which includes quantum
        cout << ALGORITHMS[algorithm_id_val] << quantum_val << endl;
    else
        cout << ALGORITHMS[algorithm_id_val] << endl;
}

// printProcesses: Prints the names of all processes.
void printProcesses()
{
    cout << "Process    ";
    for (int i = 0; i < process_count; i++)
        cout << "|  " << getProcessName(processes[i]) << "  ";
    cout << "|\n";
}

// printArrivalTime: Prints the arrival time for each process.
void printArrivalTime()
{
    cout << "Arrival    ";
    for (int i = 0; i < process_count; i++)
        printf("|%3d  ",getArrivalTime(processes[i]));
    cout<<"|\n";
}

// printServiceTime: Prints the service time for each process and calculates the mean service time.
void printServiceTime()
{
    cout << "Service    |";
    for (int i = 0; i < process_count; i++)
        printf("%3d  |",getServiceTime(processes[i]));
    cout << " Mean|\n";
}

// printFinishTime: Prints the finish time for each process.
void printFinishTime()
{
    cout << "Finish     ";
    for (int i = 0; i < process_count; i++)
        printf("|%3d  ",finishTime[i]);
    cout << "|-----|\n";
}

// printTurnAroundTime: Prints the turnaround time for each process and calculates the mean turnaround time.
void printTurnAroundTime()
{
    cout << "Turnaround |";
    int sum = 0;
    for (int i = 0; i < process_count; i++)
    {
        printf("%3d  |",turnAroundTime[i]);
        sum += turnAroundTime[i];
    }
    if((1.0 * sum / turnAroundTime.size())>=10)
		printf("%2.2f|\n",(1.0 * sum / turnAroundTime.size()));
    else
	 	printf(" %2.2f|\n",(1.0 * sum / turnAroundTime.size()));
}

// printNormTurn: Prints the normalized turnaround time for each process and calculates the mean normalized turnaround time.
void printNormTurn()
{
    cout << "NormTurn   |";
    float sum = 0;
    for (int i = 0; i < process_count; i++)
    {
        if( normTurn[i]>=10 )
            printf("%2.2f|",normTurn[i]);
        else
            printf(" %2.2f|",normTurn[i]);
        sum += normTurn[i];
    }

    if( (1.0 * sum / normTurn.size()) >=10 )
        printf("%2.2f|\n",(1.0 * sum / normTurn.size()));
	else
        printf(" %2.2f|\n",(1.0 * sum / normTurn.size()));
}

// printStats: Orchestrates the printing of all statistical metrics.
void printStatsOutput(int algorithm_id_val, int quantum_val)
{
    printAlgorithm(algorithm_id_val, quantum_val);
    printProcesses();
    printArrivalTime();
    printServiceTime();
    printFinishTime();
    printTurnAroundTime();
    printNormTurn();
}


int main() {
    cout << "=========================================\n";
    cout << " CPU SCHEDULING SIMULATOR TOOL      \n";
    cout << " (Supports FCFS, RR, SPN, SRT, HRRN, AGING)\n";
    cout << "=========================================\n\n";

    cout << "Enter number of processes: ";
    cin >> process_count;
    processes.resize(process_count);
    finishTime.resize(process_count);
    turnAroundTime.resize(process_count);
    normTurn.resize(process_count);
    for (int i = 0; i < process_count; ++i) {
        string name; int arrival, service;
        cout << "Enter name, arrival time, and service time for process " << i + 1 << ": ";
        cin >> name >> arrival >> service;
        processes[i] = make_tuple(name, arrival, service);
        processToIndex[name] = i;
        last_instant = max(last_instant, arrival + service);
    }
    // Add some buffer to last_instant for timeline display
    last_instant += 10;
    timeline = vector<vector<char>>(last_instant, vector<char>(process_count, ' '));

    cout << "Choose Algorithm:\n1. FCFS\n2. RR\n3. SPN\n4. SRT\n5. HRRN\n6. AGING\nEnter choice: ";
    int choice; cin >> choice;

    int quantum = -1; // Default quantum, only used for RR
    if (choice == 2) {
        cout << "Enter quantum for Round Robin: ";
        cin >> quantum;
    }

    clear_timeline(); // Clear timeline before running the selected algorithm

    // Execute the chosen algorithm
    switch (choice) {
        case 1: firstComeFirstServe(); break;
        case 2: roundRobin(quantum); break;
        case 3: shortestProcessNext(); break;
        case 4: shortestRemainingTime(); break;
        case 5: highestResponseRatioNext(); break;
        case 6: agingAlgorithm(); break;
        default:
            cout << "Invalid algorithm choice." << endl;
            return 1;
    }

    cout << "\nChoose Output Format:\n1. Trace\n2. Statistics\nEnter choice: ";
    int outputChoice;
    cin >> outputChoice;

    if (outputChoice == 1) {
        printTimelineOutput(); // Call the new printTimelineOutput function
    } else if (outputChoice == 2) {
        printStatsOutput(choice, quantum); // Call the new printStatsOutput function with algorithm choice and quantum
    } else {
        cout << "Invalid output choice." << endl;
    }

    return 0;
}
