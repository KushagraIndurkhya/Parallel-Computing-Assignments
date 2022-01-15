/*
Name:Kushagra Indurkhya
Roll No:CS19B1017
Assignment No:1

Implemented:
- Dynamic Allocation Method
- Static Allocation Method - 1
- Static Allocation Method - 2
- Sequential Methos (for benchmarking)
*/

#include <iostream>
#include <pthread.h>
#include <mutex>
#include <math.h>
#include <fstream>
using namespace std;

/*
Class: Counter
Constructor Counter(): Initializes the counter to 0
int getAndIncrement(): Returns the value of the counter and increments it by 1
mutex is used to lock the critical section
*/
class Counter
{
private:
    mutex *m;

public:
    int count;
    // Constructor
    Counter()
    {
        count = 0;
        m = new mutex();
    };
    // Returns the value of the counter and increments it by 1
    int getAndIncrement()
    {
        int res = 0;
        // critical section
        m->lock();
        res = count++;
        m->unlock();
        return res;
    }
};
/*
Class: Writer
Constructor Writer(string filename): creates and opens a file with the given name
void write(int n): appends the value of n in the file
*/
class Writer
{
private:
    ofstream *outfile;
    mutex *m;

public:
    Writer(){};
    Writer(string filename)
    {
        outfile = new ofstream(filename);
        m = new mutex();
    };
    // Write is used to append the value of n in the file
    void write(int s)
    {
        // Using mutex to lock the critical section
        m->lock();
        *outfile << s << " ";
        m->unlock();
    };
    // Destructor
    ~Writer()
    {
        outfile->close();
        delete outfile;
        delete m;
    };
};
/*
Class: isPrime
returns true if the number is prime else returns false
*/
bool isPrime(int n)
{
    if (n <= 1)
        return false;
    if (n == 2)
        return true;
    if (n % 2 == 0)
        return false;
    for (int i = 3; i * i <= n; i += 2)
    {
        if (n % i == 0)
            return false;
    }
    return true;
}
/*
Struct to store the data being passed to a thread in arguments
*/
struct data
{
    long end;
    int m;
    int thread_id;
};

//-------------------------Global Data-------------------------
// Global Counter for DAM
Counter c = Counter();
// Global Writer Objects for each method
Writer wd = Writer("Primes-DAM.txt");
Writer ws1 = Writer("Primes-SAM1.txt");
Writer ws2 = Writer("Primes-SAM2.txt");
Writer wseq = Writer("Primes-Seq.txt");
//----------------------------DAM-------------------------------
/*
Dynamic Allocation Method
Uses a counter to dynamically give load to threads
*/

// Function to be executed by each thread
// Arguments: limit
void *printPrime_dam(void *limit)
{
    long i = 0;
    long n = *((long *)limit);
    while (i < n)
    {
        i = c.getAndIncrement();
        if (i > n)
            break;
        if (isPrime(i))
            wd.write(i);
    }
    return NULL;
}
double dam(long limit, int m)
{
    clock_t start = clock();
    pthread_t threads[m];
    for (int i = 0; i < m; i++)
        pthread_create(&threads[i], NULL, printPrime_dam, (void *)&limit);
    for (int i = 0; i < m; i++)
        pthread_join(threads[i], NULL);
    clock_t end = clock();
    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
    return time_taken;
}
//-------------------------SAM-1-------------------------------------

/*
Static Allocation Method - 1
Each thread is assigned a banlanced load
Ex: 1,11,21,31..... are assigned to thread 1
    2,12,22,32..... are assigned to thread 2
    3,13,23,33..... are assigned to thread 3
    ....
*/
void *printPrime_sam(void *arg)
{
    long i = 0;
    data *d = (data *)arg;
    long n = d->end;
    int m = d->m;
    int my_id = d->thread_id;

    int number = 0;
    for (int i = 0; number < n; i++)
    {
        number = (i * m) + my_id;
        if (number > n)
            break;
        if (isPrime(number))
            ws1.write(number);
    }
    return NULL;
}
double sam(long limit, int m)
{
    clock_t start = clock();
    pthread_t threads[m];
    struct data d[m];
    for (int i = 0; i < m; i++)
    {
        d[i].end = limit;
        d[i].m = m;
        d[i].thread_id = i;
    }
    for (int i = 0; i < m; i++)
        pthread_create(&threads[i], NULL, printPrime_sam, (void *)&d[i]);
    for (int i = 0; i < m; i++)
        pthread_join(threads[i], NULL);

    clock_t end = clock();
    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
    return time_taken;
}
//-------------------------SAM-2-----------------------------------------
// Static Allocation Method - 2
void *printPrime_sam2(void *arg)
{
    data *d = (data *)arg;
    long n = d->end;
    int m = d->m;
    int my_id = d->thread_id;
    int number = 0;

    int i = 0; //default value
    int offset;
    if (m % 2 == 0) //Number of threads is even
        offset = 1;
    else
    {
        offset = 2;//Number of threads is odd means that either all i should be even or all i should be odd
        if (my_id % 2 == 0) //Number of threads is odd and thread id is even
            i = 1;//i should be always odd
    }
    while (number < n)
    {
        number = (i * m) + my_id;
        if (number > n)
            break;
        if (isPrime(number))
            ws2.write(number);
        i += offset;
    }

    return NULL;
}
double sam2(long limit, int m)
{
    clock_t start = clock();
    ws2.write(2);
    pthread_t threads[m];
    struct data d[m];
    if (m % 2 == 0) // Number of threads is even
    {
        for (int i = 0, j = 1; i < m; i++, j += 2)
        {
            d[i].end = limit;
            d[i].m = m * 2;
            d[i].thread_id = j;
        }
    }
    else // Number of threads is odd
    {
        for (int i = 0; i < m; i++)
        {
            d[i].end = limit;
            d[i].m = m;
            d[i].thread_id = i;
        }
    }
    for (int i = 0; i < m; i++)
        pthread_create(&threads[i], NULL, printPrime_sam2, (void *)&d[i]);
    for (int i = 0; i < m; i++)
        pthread_join(threads[i], NULL);
    clock_t end = clock();
    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
    return time_taken;
}
//----------------------------Sequential---------------------------------------------
/*
Sequential version of the program.
Args: limit: the upper limit of the range of numbers to be checked for primes
Returns the time taken.
*/
double sequential(long limit)
{
    clock_t start = clock();
    for (long i = 0; i < limit; i++)
    {
        if (isPrime(i))
            wseq.write(i);
    }
    clock_t end = clock();
    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
    return time_taken;
}
//-------------------------------Main-------------------------------------------------
int main()
{
    // Input
    int n, m;
    ifstream infile;
    infile.open("inp-params.txt");
    infile >> n >> m;
    // Calculating Limit
    long limit = long(pow(10, n));

    // Calling DAM, SAM-1, SAM-2 and Sequential and printing the time taken to a file
    ofstream time_file;
    time_file.open("Times.txt");
    time_file << dam(limit, m) << " " << sam(limit, m) << " " << sam2(limit, m) << " " << sequential(limit) << endl;
    time_file.close();

    return 0;
}