#include "test_app.cpp"
#include "thread_local.cpp"
#include "qnode.cpp"

class MCSLock : public Lock
{
    atomic<QNode *> tail;
    ThreadLocal<QNode *> *myNode;

public:
    MCSLock(int n)
    {
        tail.store(nullptr);
        myNode = new ThreadLocal<QNode *>(n);
        for (int i = 0; i < n; i++)
        {
            myNode->arr[i] = new QNode();
        }
    }
    void lock(int t_id)
    {
        QNode *qnode = myNode->get(t_id);
        QNode *pred = tail.exchange(qnode);
        if (pred != nullptr)
        {
            qnode->locked = true;
            pred->next = qnode;
            while (qnode->locked)
            {
            }
        }
    }
    void unlock(int t_id)
    {
        QNode *qnode = myNode->get(t_id);
        if (qnode->next == NULL)
        {
            if (tail.compare_exchange_strong(qnode, NULL))
                return;
            while (qnode->next == NULL)
            {
            }
        }
        qnode->next->locked = false;
        qnode->next = NULL;
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
    test = new MCSLock(n);
    call_threads(n,"MCS");
    return 0; 
}