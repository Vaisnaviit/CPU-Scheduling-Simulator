#include <bits/stdc++.h>
using namespace std;

// Constants representing two types of output modes
const string TRACE = "trace";
const string SHOW_STATISTICS = "stats";

// Global variables for scheduling simulation
int last_instant = 0, process_count = 0;

// Vector storing all processes as a tuple: (ProcessName, ArrivalTime, ServiceTime)
vector<tuple<string, int, int>> processes;

// Timeline matrix: tracks execution for each time unit of each process
// '*' means running, '.' means waiting, ' ' (blank) means no process yet
vector<vector<char>> timeline;

// Map linking process name to its index in the processes vector
unordered_map<string, int> processToIndex;

// Result containers for each process
vector<int> finishTime;     // Time when a process finishes execution
vector<int> turnAroundTime; // Turnaround = FinishTime - ArrivalTime
vector<float> normTurn;     // Normalized Turnaround = Turnaround / ServiceTime

// Names of algorithms (for printing Stats table)
const string ALGORITHMS[9] = {"", "FCFS", "RR-", "SPN", "SRT", "HRRN", "FB-1", "FB-2i", "AGING"};

// Reset/clear timeline before each scheduling algorithm runs
void clear_timeline() {
    for (int i = 0; i < last_instant; i++)
        for (int j = 0; j < process_count; j++)
            timeline[i][j] = ' ';  // Blank means nothing executed here yet
}

// Helper functions for easy tuple data access
int getArrivalTime(const tuple<string,int,int>& a) { return get<1>(a); }
int getServiceTime(const tuple<string,int,int>& a) { return get<2>(a); }
string getProcessName(const tuple<string,int,int>& a) { return get<0>(a); }

// HRRN formula: (Waiting + Service) / Service
double calculate_response_ratio(int wait_time, int service_time) {
    return (wait_time + service_time) * 1.0 / service_time;
}

// Fill remaining waiting periods with '.' after execution completes
void fillInWaitTime() {
    for (int i = 0; i < process_count; i++) {
        int arrivalTime = getArrivalTime(processes[i]);
        for (int k = arrivalTime; k < finishTime[i] && k < last_instant; k++) {
            if (timeline[k][i] != '*') timeline[k][i] = '.';
        }
    }
}

/*
 FCFS: First Come First Serve Scheduling
 Non-preemptive: jobs executed in arrival order
*/
void firstComeFirstServe() {
    int time = getArrivalTime(processes[0]);

    for (int i = 0; i < process_count; i++) {
        int arrival = getArrivalTime(processes[i]);
        int service = getServiceTime(processes[i]);

        // Process starts when CPU is free or when it arrives
        finishTime[i] = max(time, arrival) + service;

        // Calculating turnaround and normalized turnaround
        turnAroundTime[i] = finishTime[i] - arrival;
        normTurn[i] = turnAroundTime[i] * 1.0 / service;

        // Fill execution '*' in timeline
        int start = max(time, arrival);
        for (int j = start; j < finishTime[i] && j < last_instant; j++) timeline[j][i] = '*';

        // Fill waiting time '.'
        for (int j = arrival; j < start && j < last_instant; j++) timeline[j][i] = '.';

        time = finishTime[i]; // CPU moves forward
    }
    fillInWaitTime();
}

/*
 Round Robin Scheduling (Preemptive)
 Each process gets fixed quantum of CPU time

*/
void roundRobin(int quantum) {
    queue<int> q; // Queue for ready processes
    vector<int> remaining(process_count); // Remaining service time

    for (int i = 0; i < process_count; ++i)
        remaining[i] = getServiceTime(processes[i]);

    int time = 0, j = 0;

    while (time < last_instant || !q.empty() || j < process_count) {
        // Add newly arrived processes to queue
        while (j < process_count && getArrivalTime(processes[j]) <= time)
            q.push(j++);

        if (!q.empty()) {
            int idx = q.front(); q.pop();
            int run = min(quantum, remaining[idx]);

            // Mark execution in timeline
            for (int t = 0; t < run; ++t)
                if (time + t < last_instant) timeline[time + t][idx] = '*';

            time += run;
            remaining[idx] -= run;

            // Re-queue if not finished
            while (j < process_count && getArrivalTime(processes[j]) <= time)
                q.push(j++);

            if (remaining[idx] > 0) q.push(idx);
            else {
                // Process completed
                finishTime[idx] = time;
                turnAroundTime[idx] = time - getArrivalTime(processes[idx]);
                normTurn[idx] =
                    turnAroundTime[idx] * 1.0 / getServiceTime(processes[idx]);
            }
        } else {
            // If no ready process, jump to arrival of next process
            if (j < process_count) time = getArrivalTime(processes[j]);
            else time++;
        }
    }
    fillInWaitTime();
}

/*
 SPN: Shortest Process Next
 Non-preemptive. Selects process having minimum service time
*/
void shortestProcessNext() {
    vector<bool> done(process_count, false);
    int time = 0;

    while (count(done.begin(), done.end(), true) < process_count) {
        int idx = -1, minST = INT_MAX;

        // Find shortest job from ready queue
        for (int i = 0; i < process_count; ++i)
            if (!done[i] && getArrivalTime(processes[i]) <= time &&
                getServiceTime(processes[i]) < minST) {
                idx = i; minST = getServiceTime(processes[i]);
            }

        if (idx == -1) {
            // CPU idle: move time to next arrival
            int nextArrival = INT_MAX;
            for (int i = 0; i < process_count; ++i)
                if (!done[i]) nextArrival = min(nextArrival, getArrivalTime(processes[i]));
            time = nextArrival;
            continue;
        }

        // Execute job completely
        for (int t = time; t < time + minST && t < last_instant; ++t)
            timeline[t][idx] = '*';

        time += minST;
        finishTime[idx] = time;
        turnAroundTime[idx] = time - getArrivalTime(processes[idx]);
        normTurn[idx] = turnAroundTime[idx] * 1.0 / minST;
        done[idx] = true;
    }
    fillInWaitTime();
}

/*
 SRT: Shortest Remaining Time Scheduling
 Preemptive version of SPN
*/
void shortestRemainingTime() {
    vector<int> remaining(process_count);
    for (int i = 0; i < process_count; i++)
        remaining[i] = getServiceTime(processes[i]);

    vector<bool> done(process_count, false);
    int time = 0, completed = 0;

    while (completed < process_count) {
        int idx = -1, minRem = INT_MAX;

        // Find ready process with minimum remaining time
        for (int i = 0; i < process_count; ++i)
            if (!done[i] && getArrivalTime(processes[i]) <= time &&
                remaining[i] > 0 && remaining[i] < minRem) {
                idx = i; minRem = remaining[i];
            }

        if (idx == -1) {
            // Jump to next arriving process
            int nextArrival = INT_MAX;
            for (int i = 0; i < process_count; ++i)
                if (!done[i]) nextArrival = min(nextArrival, getArrivalTime(processes[i]));
            time = max(time + 1, nextArrival);
            continue;
        }

        // Execute only 1 time unit
        if (time < last_instant) timeline[time][idx] = '*';
        remaining[idx]--; time++;

        if (remaining[idx] == 0) {
            finishTime[idx] = time;
            turnAroundTime[idx] = time - getArrivalTime(processes[idx]);
            normTurn[idx] = turnAroundTime[idx] * 1.0 / getServiceTime(processes[idx]);
            done[idx] = true;
            completed++;
        }
    }
    fillInWaitTime();
}

/*
 HRRN: Highest Response Ratio Next
 Prioritizes long-waiting processes
*/
void highestResponseRatioNext() {
    vector<bool> done(process_count, false);
    int time = 0;

    while (count(done.begin(), done.end(), true) < process_count) {
        double maxRatio = -1.0; int idx = -1;

        // Select process with maximum response ratio
        for (int i = 0; i < process_count; ++i)
            if (!done[i] && getArrivalTime(processes[i]) <= time) {
                int wait = time - getArrivalTime(processes[i]);
                int service = getServiceTime(processes[i]);
                double ratio = calculate_response_ratio(wait, service);
                if (ratio > maxRatio) { maxRatio = ratio; idx = i; }
            }

        if (idx == -1) {
            // Move to next arrival
            int nextArrival = INT_MAX;
            for (int i = 0; i < process_count; ++i)
                if (!done[i]) nextArrival = min(nextArrival, getArrivalTime(processes[i]));
            time = max(time + 1, nextArrival);
            continue;
        }

        // Execute entire job
        int st = getServiceTime(processes[idx]);
        for (int t = time; t < time + st && t < last_instant; ++t)
            timeline[t][idx] = '*';

        time += st;
        finishTime[idx] = time;
        turnAroundTime[idx] = time - getArrivalTime(processes[idx]);
        normTurn[idx] = turnAroundTime[idx] * 1.0 / st;
        done[idx] = true;
    }
    fillInWaitTime();
}

/*
Aging Algorithm
 Wait increases priority so starvation doesn't happen
 Preemptive scheduling based on aging priority counter
*/
void agingAlgorithm() {
    vector<int> remaining(process_count);
    for (int i = 0; i < process_count; i++)
        remaining[i] = getServiceTime(processes[i]);

    vector<int> priority(process_count, 0);
    vector<bool> done(process_count, false);
    int time = 0;

    while (count(done.begin(), done.end(), true) < process_count) {
        int idx = -1;

        // Increase priority for every waiting job
        for (int i = 0; i < process_count; ++i)
            if (!done[i] && getArrivalTime(processes[i]) <= time) {
                priority[i]++;
                if (idx == -1 || priority[i] > priority[idx])
                    idx = i;
            }

        if (idx == -1) {
            // idle CPU
            time++;
            continue;
        }

        // Execute one unit
        if (time < last_instant) timeline[time][idx] = '*';
        remaining[idx]--; time++;

        if (remaining[idx] == 0) {
            finishTime[idx] = time;
            turnAroundTime[idx] = time - getArrivalTime(processes[idx]);
            normTurn[idx] = turnAroundTime[idx] * 1.0 / getServiceTime(processes[idx]);
            done[idx] = true;
        }
    }
    fillInWaitTime();
}

// Prints timeline trace output format (Option 1)
void printTimelineOutput() {
    cout << "Time: ";
    for (int t = 0; t < last_instant; ++t) {
        cout << t;
        if (t + 1 < last_instant) cout << " ";
    }
    cout << "\n\n";

    for (int i = 0; i < process_count; ++i) {
        cout << getProcessName(processes[i]) << ": ";
        for (int t = 0; t < last_instant; ++t) {
            char c = timeline[t][i];
            cout << (c == ' ' ? '.' : c);
            if (t + 1 < last_instant) cout << " ";
        }
        cout << "\n";
    }
}

// Functions to print statistics format table (Option 2)
void printAlgorithm(int algorithm_id_val, int quantum_val) {
    if (algorithm_id_val == 2) cout << ALGORITHMS[algorithm_id_val] << quantum_val << endl;
    else cout << ALGORITHMS[algorithm_id_val] << endl;
}
void printProcesses() {
    cout << "Process    ";
    for (int i = 0; i < process_count; i++)
        cout << "|  " << getProcessName(processes[i]) << "  ";
    cout << "|\n";
}
void printArrivalTime() {
    cout << "Arrival    ";
    for (int i = 0; i < process_count; i++)
        printf("|%3d  ", getArrivalTime(processes[i]));
    cout << "|\n";
}
void printServiceTime() {
    cout << "Service    |";
    for (int i = 0; i < process_count; i++)
        printf("%3d  |", getServiceTime(processes[i]));
    cout << " Mean|\n";
}
void printFinishTime() {
    cout << "Finish     ";
    for (int i = 0; i < process_count; i++)
        printf("|%3d  ", finishTime[i]);
    cout << "|-----|\n";
}
void printTurnAroundTime() {
    cout << "Turnaround |";
    int sum = 0;
    for (int i = 0; i < process_count; i++) {
        printf("%3d  |", turnAroundTime[i]);
        sum += turnAroundTime[i];
    }
    printf(" %2.2f|\n", (1.0 * sum / turnAroundTime.size()));
}
void printNormTurn() {
    cout << "NormTurn   |";
    float sum = 0;
    for (int i = 0; i < process_count; i++) {
        printf(" %2.2f|", normTurn[i]);
        sum += normTurn[i];
    }
    printf(" %2.2f|\n", (1.0 * sum / normTurn.size()));
}
void printStatsOutput(int algorithm_id_val, int quantum_val) {
    printAlgorithm(algorithm_id_val, quantum_val);
    printProcesses();
    printArrivalTime();
    printServiceTime();
    printFinishTime();
    printTurnAroundTime();
    printNormTurn();
}

// Driver function
int main() {
    cout << "Enter total number of processes: ";
    cin >> process_count;

    processes.resize(process_count);
    finishTime.assign(process_count, 0);
    turnAroundTime.assign(process_count, 0);
    normTurn.assign(process_count, 0.0f);

    cout << "Enter process name, arrival time, and service time for each process:\n";
    for (int i = 0; i < process_count; ++i) {
        string name; int arrival, service;
        cin >> name >> arrival >> service;

        processes[i] = make_tuple(name, arrival, service);
        processToIndex[name] = i;

        // Update total timeline size based on latest finishing possibility
        last_instant = max(last_instant, arrival + service);
    }

    last_instant += 10; // Extra padding in timeline for safe visualization
    timeline = vector<vector<char>>(last_instant, vector<char>(process_count, ' '));

    cout << "Select the Scheduling Algorithm:\n";
    cout << "1. First Come First Serve (FCFS)\n";
    cout << "2. Round Robin (RR)\n";
    cout << "3. Shortest Process Next (SPN)\n";
    cout << "4. Shortest Remaining Time (SRT)\n";
    cout << "5. Highest Response Ratio Next (HRRN)\n";
    cout << "6. Aging\n";
    cout << "Enter choice: ";
    int choice; cin >> choice;

    int quantum = -1;
    if (choice == 2) {
        cout << "Enter Time Quantum: ";
        cin >> quantum;
    }

    clear_timeline(); // reset timeline before executing selected algorithm

    //invoke chosen scheduling algorithm
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

    cout << "Select the Output Format:\n1. Trace Format\n2. Statistics Format\nEnter choice: ";
    int outputChoice; cin >> outputChoice;

    if (outputChoice == 1)
        printTimelineOutput(); // show '*'/'.' per time unit
    else if (outputChoice == 2)
        printStatsOutput(choice, quantum); // show tables
    else
        cout << "Invalid output choice." << endl;

    return 0;
}
