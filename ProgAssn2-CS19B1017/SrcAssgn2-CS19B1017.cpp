/*
Name:Kushagra Indurkhya
Roll No:CS19B1017
Assignment No:2

Implemented:
-Filter Lock
-Peterson Tree Lock
*/

//-------------------------------------------------Dependencies------------------------------------------------------------------
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

//--------------------------------------------------Base Class---------------------------------------------------------------------
/* This is the base class for all locks. */
class lock_base
{
public:
    virtual void lock(int thread_id) = 0;
    virtual void unlock(int thread_id) = 0;
};

//----------------------------------------------------Filter Lock------------------------------------------------------------------
class filter : public lock_base
{
private:
    /* data */
    int *level;
    int *victim;
    int n;

public:
    /* This is the constructor of the filter class. It initializes the level and victim arrays to 0. */
    filter(int n)
    {
        level = new int[n];
        victim = new int[n];
        this->n = n;
        for (int i = 0; i < n; i++)
            level[i] = 0;
    }
    /* This is the implementation of the Filter Lock algorithm. */
    void lock(int thread_id)
    {
        int me = thread_id;
        for (int i = 1; i < n; i++)
        { // attempt level 1
            level[me] = i;
            victim[i] = me;
            // spin while conflicts exist
            for (int k = 0; k < n; k++)
                while ((k != me) && (level[k] >= i && victim[i] == me))
                {
                }
        }
    }
    /* This function sets the level of the thread to 0. */
    void unlock(int thread_id)
    {
        level[thread_id] = 0;
    }
};

//-------------------------------------------------PETERSON_TREE_LOCK-----------------------------------------------------
class node
{
public:
    node *leftChild;
    node *rightChild;
    node *parent;
    atomic_bool *flag;
    atomic_int victim;
    int i_id;
    int j_id;

    node() {}
    node(node *par)
    {
        flag = new atomic_bool[2];
        parent = par;
        i_id = -1;
        j_id = -1;
    }
    void lock(int thread_id)
    {
        //i_id is not set yet so set it to the thread_id requesting this lock
        if (i_id == -1)
            i_id = thread_id;
        // i_id is not set but j_id is not set so set j_id to the thread_id requesting this lock
        else if (j_id == -1)
            j_id = thread_id;
        //both i_id and j_id are set
        else
        {
            //0th peterson thread is not requesting the lock
            if (flag[0].load() == false)
                i_id = thread_id;
            //1st peterson thread is not requesting the lock
            else if (flag[1].load() == false)
                j_id = thread_id;
            //both peterson threads are requesting the lock
            else
                //Wait till either frees up
                while (flag[0].load() == true && flag[1].load() == true){}       
        }

        int i;
        if (thread_id == i_id)
            i = 0;
        else
            i = 1;
        int j = 1 - i;
        flag[i].store(true);
        victim.store(i);
        while (flag[j].load() && victim.load() == i){}
    }
    void unlock(int thread_id)
    {
        // flag[thread_id].store(false);
        int me;
        if (thread_id == i_id)
            me = 0;
        else if (thread_id == j_id)
            me = 1;
        flag[me].store(false);
    }
};

class ptl : public lock_base
{
public:
    int n;
    node *root; //root node of the tree
    vector<node *> *leaf_nodes; //vector of leaf nodes
    int height; //height of the tree
    ptl(int n)
    {
        this->n = n;//number of threads
        root = new node(NULL);//root node
        vector<node *> *tree = new vector<node *>; //initialize the tree
        tree->push_back(root);//push the root node to the tree
        leaf_nodes = recursiveBuild(tree);//recursively build the tree
        height = log2(n);//calculate the height of the tree
    }

    /* Used to build the tree recursively. */
    vector<node *> *recursiveBuild(vector<node *> *nodes)
    {
        //base case
        if (nodes->size() == ((this->n) / 2))
            return nodes;
    
        vector<node *> *new_nodes = new vector<node *>;

        for (int i = 0; i < nodes->size(); i++)
        {
            node *parent = nodes->at(i);
            node *left = new node(parent);
            node *right = new node(parent);
            parent->leftChild = left;
            parent->rightChild = right;
            new_nodes->push_back(left);
            new_nodes->push_back(right);
        }
        //recursive call
        return recursiveBuild(new_nodes);
    }

    void lock(int thread_id)
    {
        node *curr = (*leaf_nodes)[thread_id / 2];
        while (curr)
        {
            curr->lock(thread_id);
            curr = curr->parent;
        }
    }

    void unlock(int thread_id)
    {
        node *curr = (*leaf_nodes)[thread_id/2];
        while (curr)
        {
            curr->unlock(thread_id);
            curr = curr->parent;
        }
    }
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
        epochMicro= curTime.tv_usec;
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
    double t1,t2;
    thread_data(int t_id, double t1,double t2)
    
    {
        thread_id = t_id;
        cs_enter_times = new vector<double>;
        cs_exit_times = new vector<double>;
        this->t1 = t1;
        this->t2 = t2;
    }
    //log the enter time and exit timea of the critical section
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
lock_base *test;
int k, lambda1, lambda2, n;
ofstream *outfile;

void TestCs(thread_data t_data)
{
    int thread_id = t_data.thread_id;
    for (int i = 0; i < k; i++)
    {
        time_stamp reqEnterTime;
        // (*outfile) << i << "th CS Entry Request At " << reqEnterTime.timeStamp << " by thread "<< thread_id <<" (mesg1)"<<endl;
        test->lock(thread_id);
        time_stamp actEnterTime;
        (*outfile) << i << "th CS Entry At\t\t" << actEnterTime.timeStamp << " by thread " << thread_id << " (mesg2)" << endl;
        sleep_for(milliseconds(int(t_data.t1*1000)));
        time_stamp reqExitTime;
        (*outfile) << i << "th CS Exit Request At\t" << reqExitTime.timeStamp << " by thread " << thread_id << " (mesg3)" << endl;
        test->unlock(thread_id);
        time_stamp actExitTime;
        // (*outfile) << i << "th CS Exit" << actExitTime.timeStamp << " by thread "<< thread_id <<" (mesg4)"<<endl;
        sleep_for(milliseconds(int(t_data.t2*1000)));
        t_data.log_entry_exit(actEnterTime.epochSeconds - reqEnterTime.epochSeconds,actExitTime.epochSeconds - reqExitTime.epochSeconds);
        // t_data.log_entry_exit(actEnterTime.epochMicro - reqEnterTime.epochMicro,actExitTime.epochMicro - reqExitTime.epochMicro);
    }
}

/* This function creates `n` threads and calls the TestCs function on each thread. */
vector<thread_data> call_threads(int n,int seed)
{
    //Generate exponentially distributed t1,t2
    default_random_engine generator(seed);
    exponential_distribution<double> sleep1(lambda1);
    exponential_distribution<double> sleep2(lambda2);
    thread t[n];
    vector<thread_data> t_data;
    for (int i = 0; i < n; i++)
    {
        t_data.push_back(thread_data(i, sleep1(generator), sleep2(generator)));
        t[i] = thread(TestCs, t_data[i]);
    }
    for (int i = 0; i < n; i++)
        t[i].join();
    return t_data;
}

int main()
{
    ifstream infile;
    infile.open("inp-params.txt");
    infile >> n >> k >> lambda1 >> lambda2;
    infile.close();
    if(n<=0 || k<=0 || lambda1<=0 || lambda2<=0 || (n & (~(n - 1)))!= n)
    {
        cout<<"Invalid input parameters"<<endl;
        return 1;
    }
    outfile = new ofstream("out.txt");

    int seed = chrono::system_clock::now().time_since_epoch().count();
    (*outfile) << "Filter Lock Output" << endl;
    test = new filter(n);
    vector<thread_data> filter_data=call_threads(n,seed);

    (*outfile) << "PTL Output" << endl;
    test = new ptl(n);
    vector<thread_data> ptl_data = call_threads(n,seed);

    cout << "------------------------------------------------------" << endl;
    cout << "Average CS Entry Time for Filter Lock: " << thread_data::get_average_cs_enter(&filter_data) << endl;
    cout << "Average CS Exit Time for Filter Lock: " << thread_data::get_average_cs_exit(&filter_data) << endl;
    cout << "Average CS Entry Time for PTL: " << thread_data::get_average_cs_enter(&ptl_data) << endl;
    cout << "Average CS Exit Time for PTL: " << thread_data::get_average_cs_exit(&ptl_data) << endl;
    cout << "------------------------------------------------------" << endl;

    outfile->close();
    return 0;
}