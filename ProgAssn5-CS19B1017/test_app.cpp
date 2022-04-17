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
#include <stdlib.h>
#include <stdio.h>
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;
mutex m_out1, m_out2;
//---------------------------------------------------------------------------------------------------------------------------------------
/* Lock is an abstract class that defines a lock and unlock method. */
class Lock
{
public:
    /* These are pure virtual function,used to make the class abstract. */
    virtual void lock(int) = 0;
    virtual void unlock(int) = 0;
};

//----------------------------------------------------TEST APPLICATION-Helper CLasses-----------------------------------------------------

/* The time_stamp class is used to store the current time in the format of `YYYY-MM-DD HH:MM:SS:MMM`. */
class time_stamp
{
public:
    time_t epochSeconds;
    time_t epochMicro;
    string timeStamp;

    time_stamp()
    {
        timeval curTime;
        char buffer[32];
        gettimeofday(&curTime, NULL);
        size_t endpos = strftime(buffer, sizeof buffer, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));
        // FOR MICROSECONDS
        snprintf(buffer + endpos, sizeof buffer - endpos, ":%03d", (int)(curTime.tv_usec / 1000));

        timeStamp = buffer;
        epochSeconds = curTime.tv_sec;
        epochMicro = curTime.tv_usec;
    }
};

/* The class `thread_data` is used to store the data of each thread. The data is used to calculate the
average time for each thread to enter and exit the critical section. */
class thread_data
{
public:
    int thread_id;
    vector<double> *cs_enter_times;
    vector<double> *cs_exit_times;
    double t1, t2;
    thread_data(int t_id, double t1, double t2)

    {
        thread_id = t_id;
        cs_enter_times = new vector<double>;
        cs_exit_times = new vector<double>;
        this->t1 = t1;
        this->t2 = t2;
    }
    // log the enter time and exit timea of the critical section
    void log_entry_exit(double entry, double exit)
    {
        (*cs_enter_times).push_back(entry);
        (*cs_exit_times).push_back(exit);
    }
    // calculate the average time for each critical section
    double get_cs_enter_avg()
    {
        double sum = 0;
        for (int i = 0; i < cs_enter_times->size(); i++)
        {
            sum += (*cs_enter_times)[i];
        }
        return sum / cs_enter_times->size();
    }
    // calculate the average time for each critical section
    double get_cs_exit_avg()
    {
        double sum = 0;
        for (int i = 0; i < cs_exit_times->size(); i++)
        {
            sum += (*cs_exit_times)[i];
        }
        return sum / cs_exit_times->size();
    }

    // static method to calculate the average time for each thread to enter the the critical section
    static double get_average_cs_enter(vector<thread_data> *threads)
    {
        double sum = 0;
        for (int i = 0; i < threads->size(); i++)
        {
            sum += (*threads)[i].get_cs_enter_avg();
        }
        return sum / threads->size();
    }
    // static method to calculate the average time for each thread to exit the the critical section
    static double get_average_cs_exit(vector<thread_data> *threads)
    {
        double sum = 0;
        for (int i = 0; i < threads->size(); i++)
        {
            sum += (*threads)[i].get_cs_exit_avg();
        }
        return sum / threads->size();
    }
};

//-----------------------------------------------------------------------------------------------------------------------------
/* Global Data Available to each thread */
Lock *test;
int k, lambda1, lambda2, n;
ofstream *outfile;

void TestCs(thread_data t_data)
{
    int thread_id = t_data.thread_id;
    for (int i = 0; i < k; i++)
    {
        auto entry_time_start = chrono::steady_clock::now();
        test->lock(thread_id);
        auto entry_time_end = chrono::steady_clock::now();
        time_stamp actEnterTime;
        (*outfile) << i << "th CS Entry At\t\t" << actEnterTime.timeStamp << " by thread " << thread_id << endl;
        sleep_for(milliseconds(int(t_data.t1 * 1000)));
        time_stamp reqExitTime;
        (*outfile) << i << "th CS Exit Request At\t" << reqExitTime.timeStamp << " by thread " << thread_id << endl;
        auto exit_time_start = chrono::steady_clock::now();
        test->unlock(thread_id);
        auto exit_time_end = chrono::steady_clock::now();
        sleep_for(milliseconds(int(t_data.t2 * 1000)));
        t_data.log_entry_exit(duration_cast<microseconds>(entry_time_end - entry_time_start).count(),
                              duration_cast<microseconds>(exit_time_end - exit_time_start).count());
    }
}

/* This function creates `n` threads and calls the TestCs function on each thread. */
void call_threads(int n, string lockAlgo)
{
    int seed = chrono::system_clock::now().time_since_epoch().count();
    // Generate exponentially distributed t1,t2
    default_random_engine generator(seed);
    exponential_distribution<double> sleep1(lambda1);
    exponential_distribution<double> sleep2(lambda2);
    thread t[n];
    vector<thread_data> t_data;
    string outfile_name = lockAlgo + "_out.txt";
    outfile = new ofstream(outfile_name);
    (*outfile) << lockAlgo + "Lock Output" << endl;
    for (int i = 0; i < n; i++)
    {
        t_data.push_back(thread_data(i, sleep1(generator), sleep2(generator)));
        t[i] = thread(TestCs, t_data[i]);
    }
    for (int i = 0; i < n; i++)
        t[i].join();

    cout << "------------------------------------------------------" << endl;
    cout << "Average CS Entry Time for " + lockAlgo + "Lock: " << thread_data::get_average_cs_enter(&t_data) << endl;
    cout << "Average CS Exit Time for " + lockAlgo + "Lock: " << thread_data::get_average_cs_exit(&t_data) << endl;
    cout << "------------------------------------------------------" << endl;

    outfile->close();
}
