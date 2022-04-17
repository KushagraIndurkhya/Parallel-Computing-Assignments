#include "test_app.cpp"

class TTASLock : public Lock
{
    atomic<bool> state;

public:
    TTASLock() : state(false) {}
    void lock(int n)
    {
        while (true)
        {
            while (state.load())
            {
            };
            if (!state.exchange(true))
                return;
        }
    }
    void unlock(int n)
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
    test = new TTASLock();
    call_threads(n,"TTAS");
    return 0; 
}
