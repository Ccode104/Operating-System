#include <iostream>
#include<bits/stdc++.h>

using namespace std;

class Process {

    public:
    string name;
    int arrivalTime;
    int cpuBurst;
    int ioBurst;
    int priority;
    int remainingTime;
    int waitingTime;
    int turnaroundTime;
    int responseTime;
    bool started;
    int cpu_exceuted_time;     // Track CPU time executed before next I/O
    int ioRemainingTime; // How much I/O time is left
    int timeForNextIO;   // Time until the next I/O
    bool is_doing_IO;           // Is the process currently in I/O?
    
    Process(string name,int arrivalTime,
    int cpuBurst,
    int ioBurst,
    int priority,
    int remainingTime,
    int waitingTime,
    int turnaroundTime,
    int responseTime,
    bool started,
    int cpu_exceuted_time,     // Track CPU time executed before next I/O
    int ioRemainingTime, // How much I/O time is left
    int timeForNextIO  , // Time until the next I/O
    bool is_doing_IO):name(name),arrivalTime(arrivalTime),
    cpuBurst(cpuBurst),
    ioBurst(ioBurst),
    priority(priority),
    remainingTime(remainingTime),
    waitingTime(waitingTime),
    turnaroundTime(turnaroundTime),
    responseTime(responseTime),
    started(started),
    cpu_exceuted_time(cpu_exceuted_time),
    ioRemainingTime(ioRemainingTime),
    timeForNextIO(timeForNextIO),
    is_doing_IO(is_doing_IO){}
};

void run_IO_processes(vector<Process*>& ioQueue, int& currentTime, queue<Process*>& Q0, queue<Process*>& Q1, queue<Process*>& Q2) {
    for (auto it = ioQueue.begin(); it != ioQueue.end();) 
    {
        Process* p = *it;
        p->ioRemainingTime--;
        if (p->ioRemainingTime == 0) {
            p->is_doing_IO = false;
            p->cpu_exceuted_time = 0;  // Reset CPU executed time for next I/O cycle
            p->cpu_exceuted_time=0; // Set the time until next I/O
            // Requeue the process based on its priority
            if (p->priority == 4) Q0.push(p);
            else if (p->priority == 3) Q1.push(p);
            else if (p->priority == 2) Q2.push(p);
            it = ioQueue.erase(it);  // Remove from I/O queue
        } else {
            ++it;
        }
    }
}

void round_robin_for_a_queue(queue<Process*>& q, int timeSlice, int& currentTime, int& contextSwitches, vector<string>& trace, vector<Process*>& ioQueue, queue<Process*>& Q0, queue<Process*>& Q1, queue<Process*>& Q2) {
    if (q.empty()) return;

    Process* currentProcess = q.front();
    q.pop();

    trace.push_back("Process " + currentProcess->name + " started at time " + to_string(currentTime));
    
    int timeSpent = 0;  // Track the time spent in this time slice

    // Execute process for the time slice or until it needs to perform I/O
    while (timeSpent < timeSlice && currentProcess->remainingTime > 0) 
    {
        // Process the I/O queue before CPU-bound processes to simulate parallel execution
        run_IO_processes(ioQueue, currentTime, Q0, Q1, Q2);
        
        if (!currentProcess->started) 
        {
            currentProcess->responseTime = currentTime - currentProcess->arrivalTime;
            currentProcess->started = true;
        }

        
        currentTime++;
        timeSpent++;
        currentProcess->remainingTime--;
        currentProcess->cpu_exceuted_time++;

        // If the process reaches the I/O interval, move it to the I/O queue
        if (currentProcess->timeForNextIO == currentProcess->cpu_exceuted_time && currentProcess->remainingTime > 0) {
            currentProcess->is_doing_IO = true;
            currentProcess->ioRemainingTime = currentProcess->ioBurst; // Set the I/O burst time
            ioQueue.push_back(currentProcess);  // Move to I/O queue
            trace.push_back("Process " + currentProcess->name + " moved to I/O at time " + to_string(currentTime));
            
            //currentTime++;
            
            currentProcess->cpu_exceuted_time=0;  // Reset the time until the next I/O burst

            break;  // Stop further execution since the process needs to perform I/O
        }
        
        
        //currentProcess->timeForNextIO--;  // Decrease the time until the next I/O burst
    }

    // If the process finishes execution
    if (currentProcess->remainingTime == 0) {
        currentProcess->turnaroundTime = currentTime - currentProcess->arrivalTime;
        trace.push_back("Process " + currentProcess->name + " finished at time " + to_string(currentTime));
    } else if (!currentProcess->is_doing_IO) {
        // If the process is not in I/O and didn't finish, re-add it to the queue
        q.push(currentProcess);
        
    }
    contextSwitches++;
    
   

    
}

int main() {
    Process P0("P0", 0, 18, 5, 4, 18, 0, 0, 0, false, 0, 5, 5, false);
    Process P1("P1", 0, 27, 6, 4, 27, 0, 0, 0, false, 0, 6, 6, false);
    Process P2("P2", 0, 44, 4, 3, 44, 0, 0, 0, false, 0, 6, 6, false);
    Process P3("P3", 0, 50, 3, 2, 50, 0, 0, 0, false, 0, 9, 9, false);
                                                               









   
    vector<Process> processes = {
        P0,  // P0 performs I/O after every 5 CPU units
        P1,  // P1 performs I/O after every 6 CPU units
        P2,  // P2 performs I/O after every 6 CPU units
        P3   // P3 performs I/O after every 9 CPU units
    };

    queue<Process*> Q0, Q1, Q2; // Queues for each priority level
    vector<Process*> ioQueue;    // I/O queue
    vector<string> trace;        // For scheduling trace
    int contextSwitches = 0;
    int currentTime = 0;

    // Add processes to appropriate queues based on priority
    for (auto& p : processes) {
        if (p.priority == 4) Q0.push(&p);
        else if (p.priority == 3) Q1.push(&p);
        else if (p.priority == 2) Q2.push(&p);
    }

    int count=0,flag=0;
    // Simulate round-robin scheduling with multilevel queues
    while (!Q0.empty() || !Q1.empty() || !Q2.empty() || !ioQueue.empty()) {
        if (!Q0.empty()) {
            flag=0;
            round_robin_for_a_queue(Q0, 3, currentTime, contextSwitches, trace, ioQueue, Q0, Q1, Q2);
        } else if (!Q1.empty()) {
            flag=0;
            round_robin_for_a_queue(Q1, 6, currentTime, contextSwitches, trace, ioQueue, Q0, Q1, Q2);
        } else if (!Q2.empty()) {
            flag=0;
            round_robin_for_a_queue(Q2, 9, currentTime, contextSwitches, trace, ioQueue, Q0, Q1, Q2);
        } else {
            // No process in the CPU queues, advance time and process I/O
            if(flag==0)
                count++;
            flag=1;
            run_IO_processes(ioQueue, currentTime, Q0, Q1, Q2);
            currentTime++;
        }
    }

    // Print the scheduling trace
    cout << "Scheduling Trace:\n";
    for (const auto& t : trace) {
        cout << t << endl;
    }

    // Print final summary
    cout << "\nFinal Summary:\n";
    for (auto& p : processes) {
        p.waitingTime = p.turnaroundTime - p.cpuBurst ;
        cout << "Process " << p.name 
             << ": Response Time = " << p.responseTime 
             << ", Turnaround Time = " << p.turnaroundTime 
             << ", Waiting Time = " << p.waitingTime << endl;
    }

    cout << "Total Context Switches: " << contextSwitches-count-1 << endl;

    return 0;
}
