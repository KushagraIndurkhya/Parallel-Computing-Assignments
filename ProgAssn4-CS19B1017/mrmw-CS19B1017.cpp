/*
Name:Kushagra Indurkhya
Roll No:CS19B1017
Assignment No:4
*/
//------------------Dependencies------------------------
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/time.h>
#include <atomic>
#include <vector>
#include <array>
#include <fstream>
#include <random>
#include <math.h>
#include <atomic>
#include <set>
#include <stdlib.h>
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;
mutex m_out1, m_out2;
//------------------------------------------------------

/* This code is defining a template class Snapshot. */
template <typename T>
class Snapshot
{
public:
    virtual void update(int, int, T) = 0;
    virtual T *snapshot(int) = 0;
};
//------------------------------------------------------
template <typename T>
/* A class that holds a value, a serial number, and a process id */
class reg_values
{
public:
    T value;
    long sn;
    int pid;
    reg_values();
    /**
     * It initializes a reg_values object with the given value, serial number, and process id.
     *
     * @param v The value to be registered.
     * @param s The serial number of the register.
     * @param p The process ID of the process that created the value.
     */
    reg_values(T v, long s, int p)
    {
        value = v;
        sn = s;
        pid = p;
    }
    T get_value()
    {
        return value;
    }
    long get_sn()
    {
        return sn;
    }
    int get_pid()
    {
        return pid;
    }
};
//----------------------Obstruction Free Implementation--------------------------------
template <typename T>
class ObstructionFreeSnapshot : public Snapshot<T>
{
public:
    int n, m;
    atomic<reg_values<T> *> *reg;
    long *sn;

    /**
     * Create a new ObstructionFreeSnapshot object
     *
     * @param m the number of registers
     * @param n the number of threads
     * @param init the initial value of the register.
     */
    ObstructionFreeSnapshot(int m, int n, T init)
    {
        this->n = n;
        this->m = m;
        reg = new atomic<reg_values<T> *>[m];
        sn = new long[n];
        for (int i = 0; i < m; i++)
            reg[i].store(new reg_values<T>(init, 0, 0));
        for (int i = 0; i < n; i++)
            sn[i] = 0;
    }
    /**
     * Update the register at position x with the value value
     *
     * @param thread_id The thread id of the thread that is updating the register.
     * @param x The index of the register to update.
     * @param value the value to be stored in the register
     *
     * @return Nothing
     */
    void update(int thread_id, int x, T value)
    {
        sn[thread_id]++;
        reg[x].store(new reg_values<T>(value, sn[thread_id], x));
        return;
    }
    /**
     * The function snapshot() takes a thread id as input and returns a pointer to an array of values.
     *
     * @param thread_id The id of the thread that is requesting a snapshot.
     *
     * @return The value of the registers.
     */
    T *snapshot(int thread_id)
    {
        reg_values<T> **aa = new reg_values<T> *[m];
        reg_values<T> **bb = new reg_values<T> *[m];
        T *result = new T[m];
        /* This code is loading the values of the registers into the array aa. */
        for (int i = 0; i < m; i++)
        {
            reg_values<T> *temp = reg[i].load();
            aa[i] = new reg_values<T>(temp->get_value(), temp->get_sn(), temp->get_pid());
        }

        while (true)
        {
            for (int i = 0; i < m; i++)
            {
                reg_values<T> *temp = reg[i].load();
                bb[i] = new reg_values<T>(temp->get_value(), temp->get_sn(), temp->get_pid());
            }
            // check if all the values are same
            bool flag = true;
            for (int i = 0; i < m; i++)
            {
                if (aa[i]->get_sn() != bb[i]->get_sn() || aa[i]->get_pid() != bb[i]->get_pid())
                {
                    flag = false;
                    break;
                }
            }
            // if all the values are same then return the values
            if (flag)
            {
                for (int i = 0; i < m; i++)
                    result[i] = bb[i]->get_value();
                return result;
            }
            // copy bb to aa
            else
            {
                for (int i = 0; i < m; i++)
                    aa[i] = bb[i];
            }
        }
    }
};
//----------------------Wait Free Implementation--------------------------------
template <typename T>
class WFSnapshot : public Snapshot<T>
{
public:
    int n, m;
    atomic<reg_values<T> *> *reg;
    vector<T *> *HELPSNAP;
    long *sn;

    /**
     * It initializes the WFSnapshot class.
     *
     * @param m number of registers
     * @param n the number of threads
     * @param init the initial value of the register.
     */
    WFSnapshot(int m, int n, T init)
    {
        this->n = n;
        this->m = m;
        reg = new atomic<reg_values<T> *>[m];
        HELPSNAP = new vector<T *>(n);
        sn = new long[n];
        for (int i = 0; i < m; i++)
        {
            reg[i].store(new reg_values<T>(init, 0, 0));
        }
        for (int i = 0; i < n; i++)
        {
            sn[i] = 0;
        }
    }
    /**
     * It updates the register with the new value.
     *
     * @param thread_id the thread id of the thread that is updating the register.
     * @param x the index of the register to be updated
     * @param value the value to be stored in the register
     *
     * @return Nothing.
     */
    void update(int thread_id, int x, T value)
    {
        sn[thread_id]++;
        reg[x].store(new reg_values<T>(value, sn[thread_id], x));
        HELPSNAP->at(thread_id) = snapshot(thread_id);
        return;
    }
    /**
     * The function is used to get the snapshot of the register values
     *
     * @param thread_id the id of the thread that is requesting a snapshot.
     *
     * @return The value of the register that has been updated.
     */
    T *snapshot(int thread_id)
    {
        set<int> *can_help = new set<int>();
        reg_values<T> **aa = new reg_values<T> *[m];
        reg_values<T> **bb = new reg_values<T> *[m];
        T *result = new T[m];
        for (int i = 0; i < m; i++)
        {
            reg_values<T> *temp = reg[i].load();
            aa[i] = new reg_values<T>(temp->get_value(), temp->get_sn(), temp->get_pid());
        }

        while (true)
        {
            for (int i = 0; i < m; i++)
            {
                reg_values<T> *temp = reg[i].load();
                bb[i] = new reg_values<T>(temp->get_value(), temp->get_sn(), temp->get_pid());
            }
            bool flag = true;
            // Checking if the snapshot is consistent
            for (int i = 0; i < m; i++)
            {
                if (aa[i]->get_sn() != bb[i]->get_sn() || aa[i]->get_pid() != bb[i]->get_pid())
                {
                    flag = false;
                    break;
                }
            }
            if (flag)
            {
                for (int i = 0; i < m; i++)
                    result[i] = bb[i]->get_value();
                return result;
            }
            /* Checking if the snapshots are the same. */
            for (int i = 0; i < m; i++)
            {
                if (aa[i]->get_sn() != bb[i]->get_sn())
                {
                    /* Checking if the process is in the can_help set. If it is not, it is added to the set. If it is, it
                    is returned. */
                    int w = bb[i]->get_pid();
                    if (can_help->find(w) == can_help->end())
                        can_help->insert(w);
                    else
                        return HELPSNAP->at(w);
                }
            }
            /* Copying the elements of bb into aa. */
            for (int i = 0; i < m; i++)
                aa[i] = bb[i];
        }
    }
};
//-------------------------Global Variables-----------------------------
atomic<bool> term;
Snapshot<int> *snapshotObj;
vector<string> *write_logs;
vector<string> *snap_logs;
default_random_engine *generator;
exponential_distribution<double> *sleep_w;
exponential_distribution<double> *sleep_s;
int nw, ns, m, k;
double uw, us;
//------------------------------------------------------
/* This class is used to generate a timestamp for logging purposes */
class time_stamp
{
public:
    time_t epochSeconds;
    time_t epochMicro;
    string timeStamp;

    /**
     * Create a timestamp string that contains the current time in the format "HH:MM:SS:mmm"
     */
    time_stamp()
    {
        timeval curTime;
        char buffer[32];
        gettimeofday(&curTime, NULL);
        size_t endpos = strftime(buffer, sizeof buffer, "%H:%M:%S", localtime(&curTime.tv_sec));
        // FOR MILLI SECONDS
        snprintf(buffer + endpos, sizeof buffer - endpos, ":%03d", (int)(curTime.tv_usec / 1000));
        // FOR MICRO SECONDS
        snprintf(buffer + endpos, sizeof buffer - endpos, ":%06d", (int)(curTime.tv_usec));

        timeStamp = buffer;
        epochSeconds = curTime.tv_sec;
        epochMicro = curTime.tv_usec;
    }
};
/* A data structure that holds the thread id and the number of threads */
class w_thread_data
{
public:
    int t_id;
    int m;

    /**
     * It creates a new thread data object.
     *
     * @param id The thread id.
     * @param m the number of threads
     */
    w_thread_data(int id, int m)
    {
        this->t_id = id;
        this->m = m;
    }
};
class s_thread_data
{
public:
    int t_id;
    int n;
    int m;
    int k;
    vector<vector<int> > *snapLogs;
    vector<int> *timeLogs;
    /**
     * It creates a thread data object.
     *
     * @param t_id thread id
     * @param n the number of threads
     * @param m the number of processes
     * @param k the number of snapshots
     */
    s_thread_data(int t_id, int n, int m, int k)
    {
        this->t_id = t_id;
        this->n = n;
        this->k = k;
        this->m = m;
        this->snapLogs = new vector<vector<int> >();
        this->timeLogs = new vector<int>();
    }
    /**
     * It logs the state of the registers at a given time.
     *
     * @param time The current time.
     * @param snapshot the array that contains the current state of the registers.
     */
    void logMe(int time, int *snapshot)
    {
        vector<int> temp;
        for (int i = 0; i < n; i++)
        {
            temp.push_back(snapshot[i]);
        }
        snapLogs->push_back(temp);
        timeLogs->push_back(time);
    }
    /**
     * Prints the time logs
     */
    void print_time_logs()
    {
        for (int i = 0; i < timeLogs->size(); i++)
        {
            cout << timeLogs->at(i) << " ";
        }
        cout << endl;
    }
    /**
     * Calculate the average of all the times in the timeLogs vector
     *
     * @return The average time of the last 10 times the function was called.
     */
    double time_average()
    {
        double sum = 0;
        for (int i = 0; i < timeLogs->size(); i++)
        {
            sum += timeLogs->at(i);
        }
        return sum / timeLogs->size();
    }
    /**
     * Returns the worst time of all the times logged
     *
     * @return The maximum value in the timeLogs vector.
     */
    double worst_time()
    {
        double max = 0;
        for (int i = 0; i < timeLogs->size(); i++)
        {
            if (timeLogs->at(i) > max)
            {
                max = timeLogs->at(i);
            }
        }
        return max;
    }
};

/**
 * Update the value at the location
 *
 * @param t_data A pointer to the thread data structure.
 */
void writer(w_thread_data *t_data)
{
    int v, l, t_id;
    int m = t_data->m;
    t_id = t_data->t_id;
    long t1;
    while (!term) // Execute until term flag is set to true
    {
        v = rand() % 1000; // Get a random integer value
        l = rand() % m;    // Get a random location in the range 1..M
        // Update the value at the location
        snapshotObj->update(t_id, l, v); // Update the value at the location
        // Log the update
        string log = "Thread " + to_string(t_id) + " wrote " + to_string(v) + " at location " + to_string(l + 1) + " at " + time_stamp().timeStamp + "\n";
        m_out1.lock();
        write_logs->push_back(log);
        m_out1.unlock();
        // Sleep for a random amount of time
        t1 = long(sleep_w->operator()(*generator));
        sleep_for(microseconds(t1));
    }
}
/**
 * This function takes a snapshot of the current state of the system
 *
 * @param data the thread data
 */
void take_snapshot(s_thread_data *data)
{
    int i = 0;
    long t2;
    int k = data->k;
    int t_id = data->t_id;
    int m = data->m;
    while (i < k)
    {
        auto beginCollect = chrono::steady_clock::now();
        // Take a snapshot
        auto ithSnap = snapshotObj->snapshot(data->t_id);
        // Log the snapshot
        string temp = "";
        for (int i = 0; i < m; i++)
            temp.append("l" + to_string(i + 1) + "-" + to_string(ithSnap[i]) + " ");
        string log = "Snapshot Thread " + to_string(t_id) + "'s snapshot: " + temp + " at time " + time_stamp().timeStamp + "\n";
        auto endCollect = chrono::steady_clock::now();
        auto diffCollect = chrono::duration_cast<chrono::microseconds>(endCollect - beginCollect).count();
        // Log the time taken to collect the snapshot
        data->logMe(diffCollect, ithSnap);
        m_out2.lock();
        snap_logs->push_back(log);
        m_out2.unlock();
        // Sleep for a random time
        t2 = long(sleep_s->operator()(*generator));
        sleep_for(microseconds(t2));
        i++;
    }
}

/**
 * This function creates a vector of threads, each of which is responsible for taking a snapshot of the
 * system
 *
 * @param snapObj The Snapshot object that will be used to take snapshots.
 * @param WLogFileName The name of the file where the write logs will be stored.
 * @param SLogFileName The name of the file where the snapshots will be logged.
 *
 * @return The average time taken to take k snapshots and the worst time taken to take k snapshots.
 */
pair<double, double> call_threads(Snapshot<int> *snapObj, string WLogFileName, string SLogFileName)
{
    snapshotObj = snapObj;
    term = false;
    vector<thread> w_threads;
    vector<thread> s_threads;
    vector<w_thread_data *> w_data;
    vector<s_thread_data *> s_data;

    write_logs = new vector<string>();
    snap_logs = new vector<string>();

    // Create the write threads
    for (int i = 0; i < nw; i++)
    {
        w_thread_data *temp = new w_thread_data(i, m);
        w_data.push_back(temp);
        w_threads.push_back(thread(writer, w_data[i]));
    }
    // Create the snapshot threads
    for (int i = 0; i < ns; i++)
    {
        s_thread_data *temp = new s_thread_data(i, nw, m, k);
        s_data.push_back(temp);
        s_threads.push_back(thread(take_snapshot, s_data[i]));
    }
    // Join the Snapshot threads
    for (int i = 0; i < ns; i++)
        s_threads[i].join();
    // Terminate and Join the Write threads
    term = true;
    for (int i = 0; i < nw; i++)
        w_threads[i].join();

    // Logging
    ofstream WLogFile, SLogFile;
    WLogFile.open(WLogFileName);
    for (int i = 0; i < write_logs->size(); i++)
        WLogFile << write_logs->at(i);
    WLogFile.close();
    SLogFile.open(SLogFileName);
    for (int i = 0; i < snap_logs->size(); i++)
        SLogFile << snap_logs->at(i);
    SLogFile.close();
    // Calculating Averages and Worst Times
    double overall_worst_time = 0;
    double overall_time_average = 0;
    for (int i = 0; i < s_data.size(); i++)
    {
        double avg = s_data[i]->time_average();
        double worst = s_data[i]->worst_time();
        overall_time_average += avg;
        overall_worst_time = max(overall_worst_time, worst);
        // cout<<"Thread "<<s_data[i]->t_id<<" took\t"<<avg<<"\tmicroseconds to take "<<k<<" snapshots with worst time of "<<worst<<" microseconds"<<endl;
    }
    overall_time_average /= s_data.size();
    delete write_logs;
    delete snap_logs;

    return pair<double, double>(overall_time_average, overall_worst_time);
}

/**
 * It calls the two snapshot implementations and prints the average and worst time taken by each
 *
 * @return The average time taken by the snapshot and the worst time taken by the snapshot.
 */
int main()
{
    ifstream infile;
    infile.open("inp-params.txt");
    infile >> nw >> ns >> m >> uw >> us >> k;
    infile.close();

    generator = new default_random_engine(chrono::system_clock::now().time_since_epoch().count());
    sleep_w = new exponential_distribution<double>(1 / uw);
    sleep_s = new exponential_distribution<double>(1 / us);

    pair<double, double> res;
    res = call_threads(new WFSnapshot<int>(m, nw, 0), "write_log_file_WF.txt", "snap_log_file_WF.txt");
    cout << "Average time taken by WF snapshot is " << res.first << " microseconds and worst time is " << res.second << " microseconds" << endl;
    res = call_threads(new ObstructionFreeSnapshot<int>(m, nw, 0), "write_log_file_Of.txt", "snap_log_file_Of.txt");
    cout << "Average time taken by Of snapshot is " << res.first << " microseconds and worst time is " << res.second << " microseconds" << endl;

    return 0;
}