/*
Name:Kushagra Indurkhya
Roll No:CS19B1017
Assignment No:3

Implemented:
-Atomic SRSW
-Atomic MRSW
-Atomic MRMW
-Test application for Atomic MRMW
*/

//-------------------------------------------------Dependencies--------------------------------
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/time.h>
#include <atomic>
#include <vector>
#include <fstream>
#include <random>
#include <math.h>
using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

#define MAX_THREADS 100

//---------------------------------------------------------------------------------------------
/* The above code is defining a class template called Register. */
template <typename T>
class Register
{
public:
    /* Defining a pure virtual function. */
    virtual T read(int) = 0;
    /* Defining a pure virtual function. */
    virtual void write(T, int) = 0;
};
//---------------------------------------------------------------------------------------------
/* This code is for implementing ThreadLocal functionality for class members. */
template <typename T>
class ThreadLocal
{
public:
    T *arr;
    /**
     * Create an array of type T and initialize it to the default value of T
     *
     * @param n the number of threads that will be using the array.
     */
    ThreadLocal(int n)
    {
        arr = new T[n];
    }
    /**
     * Create an array of type T and initialize it with the value val
     *
     * @param val The value that will be assigned to each thread-local variable.
     * @param n the number of threads that will use this ThreadLocal object.
     */
    ThreadLocal(T val, int n)
    {
        arr = new T[n];
        for (int i = 0; i < n; i++)
            arr[i] = val;
    }
    /**
     * Create an array of type T for each thread
     */
    ThreadLocal()
    {
        arr = new T[MAX_THREADS];
    }

    /**
     * Create an array of type T and initialize it with the value val
     *
     * @param val The value to be assigned to each thread.
     */
    ThreadLocal(T val)
    {
        arr = new T[MAX_THREADS];
        for (int i = 0; i < MAX_THREADS; i++)
            arr[i] = val;
    }
    /**
     * Set the value of the element at index id to val
     *
     * @param id The index of the array element to set.
     * @param val The value to be set.
     */
    void set(int id, T val)
    {
        arr[id] = val;
    }
    /**
     * Return the value of the element with the given id
     *
     * @param id The id of the element you want to get.
     *
     * @return The object at the given index.
     */
    T get(int id)
    {
        return arr[id];
    }
};
//---------------------------------------------------------------------------------------------
/* This is a template class that is used to store a value and a timestamp. */
template <typename T>
class StampedValue
{
public:
    T value;
    long stamp;
    StampedValue() {}
    /**
     * Constructs a StampedValue object with the given initial value and stamp
     *
     * @param init The initial value of the StampedValue.
     */
    StampedValue(T init)
    {
        this->value = init;
        stamp = 0;
    }

    /**
     * Create a new StampedValue object with the given stamp and value
     *
     * @param stamp The time at which the value was recorded.
     * @param value The value of the StampedValue.
     */
    StampedValue(long stamp, T value)
    {
        this->value = value;
        this->stamp = stamp;
    }

    static StampedValue<T> max(StampedValue<T> x, StampedValue<T> y);
    static StampedValue<T> MIN_VALUE;
};

/* This is a template function that takes two StampedValue<T> objects and returns the one with the
highest timestamp. */
template <typename T>
StampedValue<T> StampedValue<T>::max(StampedValue<T> x, StampedValue<T> y)
{
    {
        if (x.stamp > y.stamp)
        {
            return x;
        }
        else
        {
            return y;
        }
    }
}
template <typename T>
StampedValue<T> StampedValue<T>::MIN_VALUE = StampedValue<T>(0);

//---------------------------------------------------------------------------------------------
template <typename T>
class AtomicSRSWRegister : public Register<T>
{
    ThreadLocal<long> lastStamp;
    ThreadLocal<StampedValue<T> > lastRead;
    StampedValue<T> r_value; // regular SRSW timestamp-value pair
public:
    AtomicSRSWRegister(){};
    /**
     * It initializes a register with a given initial value.
     *
     * @param init the initial value of the register
     * @param n the number of threads that will be using this register.
     */
    AtomicSRSWRegister(T init, int n)
    {
        r_value = StampedValue<T>(init);
        lastStamp = ThreadLocal<long>(0, n);
        lastRead = ThreadLocal<StampedValue<T> >(r_value, n);
    }
    /**
     * Read the value of the variable with id `t_id` and return the maximum of the value read and the last
     * value read by the thread with id `t_id`
     *
     * @param t_id The transaction id.
     *
     * @return The value of the variable at the time of the read.
     */
    T read(int t_id)
    {
        StampedValue<T> value = r_value;
        StampedValue<T> last = lastRead.get(t_id);
        StampedValue<T> result = StampedValue<T>::max(value, last);
        lastRead.set(t_id, result);
        return result.value;
    }
    /**
     * Write a value to the
     * buffer, and update the lastStamp
     *
     * @param v the value to be written
     * @param t_id The thread id.
     */
    void write(T v, int t_id)
    {
        long stamp = lastStamp.get(t_id) + 1;
        r_value = StampedValue<T>(stamp, v);
        lastStamp.set(t_id, stamp);
    }
};

//---------------------------------------------------------------------------------------------

template <typename T>
class AtomicMRSWRegister : public Register<T>
{
    ThreadLocal<long> lastStamp;
    AtomicSRSWRegister<StampedValue<T> > **a_table; // each entry is SRSW atomic
    int readers;

public:
    AtomicMRSWRegister(){};
    /**
     * The constructor creates a table of AtomicSRSWRegister objects, each of which is initialized to the
     * same value
     *
     * @param init The initial value of the register.
     * @param readers the number of readers.
     */
    AtomicMRSWRegister(T init, int readers)
    {
        this->lastStamp = ThreadLocal<long>(0, readers);
        StampedValue<T> value = StampedValue<T>(init);
        AtomicSRSWRegister<StampedValue<T> > val = AtomicSRSWRegister<StampedValue<T> >(value, readers);
        this->readers = readers;

        a_table = new AtomicSRSWRegister<StampedValue<T> > *[readers];
        for (int i = 0; i < readers; i++)
            a_table[i] = new AtomicSRSWRegister<StampedValue<T> >[readers];
        for (int i = 0; i < readers; i++)
            for (int j = 0; j < readers; j++)
                a_table[i][j] = val;
    }
    /**
     * The function reads the value from the table and then reads the values from all the other readers and
     * returns the maximum of the values
     *
     * @param t_id The thread id.
     *
     * @return The value of the maximum of the values read by the readers.
     */
    T read(int t_id)
    {
        int me = t_id;
        AtomicSRSWRegister<StampedValue<T> > value = a_table[me][me];
        StampedValue<T> result = value.read(t_id);
        for (int i = 0; i < readers; i++)
        {
            if (i != me)
            {
                result = StampedValue<T>::max(result, a_table[i][me].read(t_id));
            }
        }
        value = AtomicSRSWRegister<StampedValue<T> >(result, readers);
        for (int i = 0; i < readers; i++)
        {
            a_table[me][i].write(value.read(t_id), t_id);
        }

        return value.read(t_id).value;
    }
    /**
     * Write a value to the table
     *
     * @param v the value to be written
     * @param t_id the transaction id
     */
    void write(T v, int t_id)
    {
        long stamp = lastStamp.get(t_id) + 1;
        lastStamp.set(t_id, stamp);
        StampedValue<T> value = StampedValue<T>(stamp, v);
        for (int i = 0; i < readers; i++)
            a_table[i][i].write(value, t_id);
    }
};
//---------------------------------------------------------------------------------------------
template <typename T>
class AtomicMRMWRegister : public Register<T>
{
    int capacity;

public:
    AtomicMRSWRegister<StampedValue<T> > *a_table; // array of atomic MRSW registers
    AtomicMRMWRegister(){};
    /**
     * It initializes the AtomicMRMWRegister with the given capacity.
     *
     * @param init The initial value of the register.
     * @param capacity The number of registers in the array.
     */
    AtomicMRMWRegister(T init, int capacity)
    {
        this->capacity = capacity;

        StampedValue<T> value = StampedValue<T>(init);
        a_table = new AtomicMRSWRegister<StampedValue<T> >[capacity];
        for (int i = 0; i < capacity; i++)
            a_table[i] = AtomicMRSWRegister<StampedValue<T> >(value, capacity);
    }
    /**
     * Write a value to the table if the value is greater than the current maximum value in the table
     *
     * @param value the value to be written
     * @param t_id the id of the thread that is trying to write to the table.
     */
    void write(T value, int t_id)
    {
        int me = t_id;
        StampedValue<T> max = StampedValue<T>::MIN_VALUE;
        for (int i = 0; i < this->capacity; i++)
            max = StampedValue<T>::max(max, a_table[i].read(t_id));
        // cout<<"debug: "<<max.value<<max.stamp<<endl;
        a_table[me].write(StampedValue<T>(max.stamp + 1, value), t_id);
    }
    T read(int t_id)
    {
        StampedValue<T> max = StampedValue<T>::MIN_VALUE;
        for (int i = 0; i < this->capacity; i++)
        {
            max = StampedValue<T>::max(max, a_table[i].read(t_id));
            // cout<<"max: "<<max.stamp<<endl;
        }
        return max.value;
    }
};
//---------------------------------------------------------------------------------------------
/**
 * Given a probability p, return true with probability p
 * 
 * @param p the probability of a read operation
 * 
 * @return A boolean value.
 */
bool is_read(double p)
{
    return (rand() % 100) < (p * 100);
}
//-----------------------------------------Time Stamp----------------------------------------------------
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
        // FOR MILLI SECONDS
        snprintf(buffer + endpos, sizeof buffer - endpos, ":%03d", (int)(curTime.tv_usec / 1000));
        // FOR MICRO SECONDS
        snprintf(buffer + endpos, sizeof buffer - endpos, ":%06d", (int)(curTime.tv_usec));

        timeStamp = buffer;
        epochSeconds = curTime.tv_sec;
        epochMicro = curTime.tv_usec;
    }
};
//----------------------------------------Global Variables-----------------------------------------------------
atomic<int> shared;
AtomicMRMWRegister<int> reg;
int n, k;
double p, lambda;
ofstream *out;
mutex m;
//--------------------------------------Thread Data Class-------------------------------------------------------
class thread_data
{
public:
    int thread_id;
    int n;
    int k;
    double p;
    double lambda;
    vector<string> *logs;
    vector<double> *write_calls;
    vector<double> *read_calls;
    long avg_write_calls;
    long avg_read_calls;
    long avg_read_write;
    thread_data(){};
    /**
     * It creates a thread_data object with the given parameters.
     *
     * @param id the thread id
     * @param k the number of threads
     * @param p the probability of a write operation
     * @param l the average number of requests per second
     */
    thread_data(int id, int k, double p, double l)
    {
        this->thread_id = id;
        this->k = k;
        this->p = p;
        this->lambda = l;
        this->logs = new vector<string>;
        this->write_calls = new vector<double>;
        this->read_calls = new vector<double>;
    }
    /**
     * Logs a string and the time it was called
     *
     * @param s the string to be logged
     * @param time the time of the log call
     * @param is_read true if the log is a read, false if it is a write
     */
    void logMe(string s, long time, bool is_read)
    {
        logs->push_back(s);
        if (is_read)
            read_calls->push_back(time);
        else
            write_calls->push_back(time);
    }
    /**
     * Prints the average number of write and read calls for the thread
     */
    void print_average()
    {
        long sum_write = 0;
        long sum_read = 0;
        long sum_read_write = 0;
        for (int i = 0; i < write_calls->size(); i++)
        {
            sum_write += write_calls->at(i);
            sum_read_write += write_calls->at(i);
        }
        for (int i = 0; i < read_calls->size(); i++)
        {
            sum_read += read_calls->at(i);
            sum_read_write += read_calls->at(i);
        }
        if (write_calls->size() > 0)
            avg_write_calls = sum_write / write_calls->size();
        else
            avg_write_calls = 0;
        if (read_calls->size() > 0)
            avg_read_calls = sum_read / read_calls->size();
        else
            avg_read_calls = 0;
        avg_read_write = sum_read_write / (write_calls->size() + read_calls->size());
        
        cout << "Thread " << thread_id << " avg_write_calls: " << avg_write_calls << " avg_read_calls: " << avg_read_calls << " avg_write_read_calls: "<< avg_read_write << endl;
    }
};

//---------------------------------------------------------------------------------------------
void TestAtomic(thread_data *t_data)
{

    int lvar;
    int id = t_data->thread_id;
    int k = t_data->k;
    for (int i = 0; i < k; i++)
    {
        bool is_read_call = is_read(p);
        auto start = chrono::high_resolution_clock::now();
        time_stamp reqTime = time_stamp();
        m.lock();
        (*out) << i << "th action requested at " << reqTime.timeStamp << "by thread " << id << endl;
        m.unlock();
        if (is_read_call)
        {
            lvar = reg.read(id);
            m.lock();
            (*out) << "read:\t" << lvar << endl;
            m.unlock();
        }
        else
        {

            lvar = k * id;
            m.lock();
            (*out) << "write:\t" << lvar << endl;
            m.unlock();
            reg.write(lvar, id);
        }
        time_stamp resTime = time_stamp();
        auto end = chrono::high_resolution_clock::now();
        m.lock();
        (*out) << i << "th action completed at " << resTime.timeStamp << "by thread " << id << endl;
        m.unlock();
        t_data->logMe(reqTime.timeStamp + " " + resTime.timeStamp, chrono::duration_cast<chrono::microseconds>(end - start).count(), is_read_call);
        sleep_for(milliseconds(int(1000 * t_data->lambda)));
    }
}

void TestAtomic_std(thread_data *t_data)
{

    int lvar;
    int id = t_data->thread_id;
    int k = t_data->k;
    for (int i = 0; i < k; i++)
    {
        bool is_read_call = is_read(p);
        auto start = chrono::steady_clock::now();
        time_stamp reqTime = time_stamp();
        m.lock();
        (*out) << i << "th action requested at " << reqTime.timeStamp << " by thread " << id << endl;
        m.unlock();
        if (is_read_call)
        {
            lvar = shared.load();
            m.lock();
            (*out) << "read:\t" << lvar << endl;
            m.unlock();
        }
        else
        {
            lvar = k * id;
            m.lock();
            (*out) << "write:\t" << lvar << endl;
            m.unlock();
            shared.store(lvar);
        }
        time_stamp resTime = time_stamp();
        auto end = chrono::steady_clock::now();
        m.lock();
        (*out) << i << "th action completed at " << resTime.timeStamp << " by thread " << id << endl;
        m.unlock();
        t_data->logMe(reqTime.timeStamp + " " + resTime.timeStamp, chrono::duration_cast<chrono::microseconds>(end - start).count(), is_read_call);
        sleep_for(milliseconds(int(1000 * t_data->lambda)));
    }
}
void call_test_app(int n, int k, double p, double lambda, void (*TestAtomic)(thread_data *), int seed)
{
    thread_data *t_data = new thread_data[n];
    default_random_engine generator(seed);
    exponential_distribution<double> sleep1(lambda);
    thread t[n];
    for (int i = 0; i < n; i++)
        t_data[i] = thread_data(i, k, p, sleep1(generator));
    for (int i = 0; i < n; i++)
        t[i] = thread(TestAtomic, &t_data[i]);
    for (int i = 0; i < n; i++)
        t[i].join();

    long sum_write = 0;
    long sum_read = 0;
    long sum_write_read=0;
    for (int i = 0; i < n; i++)
    {
        t_data[i].print_average();
        sum_write += t_data[i].avg_write_calls;
        sum_read += t_data[i].avg_read_calls;
        sum_write_read += t_data[i].avg_read_write;
    }
    cout << "avg_write_calls: " << sum_write / n << " avg_read_calls: " << sum_read / n << " avg_read_write_time: " << sum_write_read/n << endl;
}
//---------------------------------------------------------------------------------------------
int main()
{
    ifstream infile;
    infile.open("inp-params.txt");
    infile >> n >> k >> lambda >> p;
    infile.close();
    out = new ofstream("out.txt");
    int seed = chrono::system_clock::now().time_since_epoch().count();
    shared = 0;
    reg = AtomicMRMWRegister<int>(0, n);
    cout << "----------------------------Custom Atomic MRMW Implementation----------------------------------------------------" << endl;
    call_test_app(n, k, p, seed, TestAtomic, lambda);
    cout << "----------------------------Standard C++ Atoimic Implementation----------------------------------------------------" << endl;
    call_test_app(n, k, p, seed, TestAtomic_std, lambda);
    return 0;
}
