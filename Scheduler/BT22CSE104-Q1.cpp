#include <iostream>
#include<bits/stdc++.h>

using namespace std;

class Job {
public:
    string process_name;
    int arrivalTime;
    int cpuBurst;
    int io_burst_time;
    int io_burst_time_org;
    int io_frequency;
    int remaining_cpu_time;
    int time_to_next_io;
    int priority;
    int response_time;
    int turnaround_time;
    bool inside_io;
    bool completed;

    Job(string name, int arrival, int cpu_burst, int io_burst, int io_freq)
        : process_name(name),
        arrivalTime(arrival), 
        cpuBurst(cpu_burst),
        io_burst_time(io_burst),
        io_burst_time_org(io_burst_time),
        io_frequency(io_freq), 
        remaining_cpu_time(cpu_burst),
        time_to_next_io(io_freq), 
        priority(0), 
        response_time(-1), 
        turnaround_time(0),
        inside_io(false), completed(false){}
};

// Comparator for the priority queue to sort by priority
struct ComparePriority 
{
    bool operator()(const Job* a, const Job* b) 
    {
        // Higher priority first, if priorities are equal, return the one with the earlier arrival time
        return (a->priority < b->priority) || (a->priority == b->priority && a->arrivalTime > b->arrivalTime);
    }
};

class Scheduler {
private:
    vector<Job> jobs;
    priority_queue<Job*, vector<Job*>, ComparePriority> ready_queue;
    vector<Job*> io_queue;
    int time;
    int alpha;
    int beta;
    int context_switches;
    Job* current_job;

public:
    Scheduler(vector<Job> jobs_list, int alpha_val, int beta_val)
        :  jobs(jobs_list),
           alpha(alpha_val),
           beta(beta_val),
           time(0), 
           context_switches(0), 
           current_job(nullptr) {}

    // Updates priorities for the ready queue and the current job
    void update_priorities() {
        
        vector<Job*> temp;

        while (!ready_queue.empty())
         {
            Job* job = ready_queue.top();
           
            ready_queue.pop();
            job->priority += alpha; // Increase priority in the ready queue
            temp.push_back(job);
        }
        // Reinsert all jobs back into the priority queue
        for (auto& job : temp) {
            ready_queue.push(job);
        }

        if (current_job != nullptr) {
            current_job->priority += beta; // Increase priority for the running job
        }
    }

    // Adds a job to the ready queue
    void add_to_ready_queue(Job* job) {
        job->priority = 0;  // Reset priority when a job is added to the ready queue
        ready_queue.push(job);
    }

    // Picks the next job from the ready queue
    Job* pick_next_job() 
    {
        //No ready state process
        if (ready_queue.empty())
        {
            return nullptr;
        }

        //Pick highest priority job
        Job* next_job = ready_queue.top();

        //Remove it from the ready state
        ready_queue.pop();

        //Return it as running state job
        return next_job;
    }

    // Updates jobs in the I/O queue
    void update_io_jobs() 
    {
        //List of jobs who have just completed IO
        vector<Job*> completed_io_jobs;

        //Loopthrough the jobs in io_queue OR blocked state
        for (auto& job : io_queue) 
        {
            //Reduce the io_burst_time i.e the remaining io_time
            job->io_burst_time--;

            if (job->io_burst_time <= 0) 
            {
                 // I/O burst is done
                completed_io_jobs.push_back(job);
                job->inside_io = false;
                job->time_to_next_io = job->io_frequency; // Reset time to next I/O
                job->io_burst_time = job->io_burst_time_org; // Correctly reset I/O burst time
            }
        }

        // Move completed I/O jobs back to the ready queue
        for (auto& job : completed_io_jobs) {
            io_queue.erase(remove(io_queue.begin(), io_queue.end(), job), io_queue.end());
            add_to_ready_queue(job);
        }
    }

    // Prints the state of the scheduler at the current time
    void print_trace() {

        //Print the current time
        cout << "Time : " << time << endl;

        // Current running job
        if (current_job != nullptr) {
            cout << "  Running process : " << current_job->process_name << endl;
            cout << "  Remaining CPU time : " << current_job->remaining_cpu_time << endl;
            cout << "  Priority of the process : " << current_job->priority << endl;
        } else {
            cout << "  Running: None / Scheduling some other process(Context Switch)" << endl;
        }

        // Ready queue
        cout << " Processes in the Ready Queue: ";
        vector<Job*> temp;
        while (!ready_queue.empty()) {
            Job* job = ready_queue.top();
            ready_queue.pop();
            cout << job->process_name << "(Priority=" << job->priority << ") ";
            temp.push_back(job);
        }
        cout << endl;
        // Reinsert all jobs back into the priority queue
        for (auto& job : temp) {
            ready_queue.push(job);
        }

        // I/O queue
        cout << " Processes in I/O Queue: ";
        for (auto& job : io_queue) {
            cout << job->process_name << " ";
        }
        cout << endl;

        cout << "--------------------------" << endl;
    }

    // Main scheduling function
    void schedule() 
    {
        //Loop for the timeline where all jobs have not completed yet
        while (any_remaining_jobs()) 
        {

            // Add newly arrived jobs to the ready queue
            for (auto& job : jobs) 
            {
                //If the job has just arrived OR a job comes after completing IO
                if (job.arrivalTime == time && !job.completed && !job.inside_io) {
                    add_to_ready_queue(&job);
                }
            }

            // Update I/O jobs and priorities
            update_io_jobs();
            update_priorities();

            // If there's no current job or the current job is finished, pick the next job
            if (current_job == nullptr || current_job->remaining_cpu_time <= 0) 
            {
                //Current Job Completed
                if (current_job != nullptr && current_job->remaining_cpu_time <= 0) 
                {
                    //Flag
                    current_job->completed = true;
                    
                    //Turnaround time
                    current_job->turnaround_time = time - current_job->arrivalTime;
                    
                    //No current Job now
                    current_job = nullptr; // Reset current job
                }

                current_job = pick_next_job();

                //The current job is starting to run after IO / Arrival 
                if (current_job != nullptr) 
                {
                    // Set the response time if the job is starting for the first time(Arrival case not IO)
                    if (current_job->response_time==-1) 
                    {
                        current_job->response_time = time - current_job->arrivalTime;
                    }
                    context_switches++;
                }
            }

            // Execute the current job
            if (current_job != nullptr) 
            {
                //Reduce remaining cpu time
                current_job->remaining_cpu_time--;

                //Reduce the time for next IO
                current_job->time_to_next_io--;

                // If the job needs to perform I/O, move it to the I/O queue
                if (current_job->time_to_next_io <= 0 && current_job->remaining_cpu_time > 0) 
                {
                    //Flag
                    current_job->inside_io = true;
                    
                    //Blocked State
                    io_queue.push_back(current_job);
                    
                    //No running job now
                    current_job = nullptr; // Job is in I/O, so reset current job
                }
            }

/*
            // Increment waiting time for jobs in the ready queue
            vector<Job*> temp;
            while (!ready_queue.empty()) {
                Job* job = ready_queue.top();
                ready_queue.pop();
                job->waiting_time++; // Increment waiting time
                temp.push_back(job);
            }
            // Reinsert all jobs back into the priority queue
            for (auto& job : temp) {
                ready_queue.push(job);
            }
*/
            // Print the scheduling trace
            print_trace();
            
            // Move to the next time unit
            time++;
        }

        /*Accomodate the corner case of last job*/
        //Turnaround time
        current_job->turnaround_time = time - current_job->arrivalTime +1;
                    
        //No current Job now
        current_job = nullptr; // Reset current job
        
        print_trace();
        
        // Print the scheduling summary
        print_summary();
    }

    // Check if there are any jobs left to process
    bool any_remaining_jobs() {
        for (auto& job : jobs) 
        {
            if (job.remaining_cpu_time > 0 || job.inside_io) return true;
        }
        return false;
    }

    // Prints the summary of the scheduling
    void print_summary() {
        cout << "Total context switches: " << context_switches << endl;
        for (const auto& job : jobs) {
            cout << "Process " << job.process_name << ":\n";
            cout << "  Response Time: " << job.response_time << endl;
            cout << "  Turnaround Time: " << job.turnaround_time << endl;
            cout << "  Waiting Time: " << job.turnaround_time - job.cpuBurst << endl;
        }
    }
};

int main() 
{

    //Input the process data

    //Process Name, Arrival Time, Burst Time, IO Burst time, IO freq(time for next io), Total CPU time(simply burst time) 
    vector<Job> jobs = 
    {
        Job("P0", 0, 18, 5, 5),
        Job("P1", 3, 27, 6, 6),
        Job("P2", 8, 44, 4, 6),
        Job("P3", 12, 50, 3, 9)
    };

    //Rate at which the priority of ready processes increases
    int alpha = 1;

    //Rate at which the priority of running process increases    
    int beta = 2;

    //Initialise the scheduler
    Scheduler scheduler(jobs, alpha, beta);

    //Call the scheduler
    scheduler.schedule();

    return 0;
}
