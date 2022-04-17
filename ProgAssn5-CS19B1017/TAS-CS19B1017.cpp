#include "test_app.cpp"

class TASLock : public Lock
{
    atomic<bool> state;

public:
    TASLock() : state(false) {}
    void lock(int t_id)
    {

        while (state.exchange(true))
        {
        }
    }
    void unlock(int t_id)
    {
        state.store(false);
    }
};



int main()
{
    ifstream infile;
    infile.open("inp-params.txt");
    infile >> n >> k >> lambda1 >> lambda2;
    infile.close();
    if (n <= 0 || k <= 0 || lambda1 <= 0 || lambda2 <= 0)
    {
        cout << "Invalid input parameters" << endl;
        return 1;
    }
    test = new TASLock();
    call_threads(n,"TAS");
    return 0; 
}