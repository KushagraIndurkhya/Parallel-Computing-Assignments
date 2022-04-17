#include "test_app.cpp"
#include "thread_local.cpp"
#include "qnode.cpp"

class CLHLock : public Lock
{
    atomic<QNode *> tail;
    ThreadLocal<QNode *>* myNode;
    ThreadLocal<QNode *>* myPred;

public:
    CLHLock(int n)
    {
        tail.store(new QNode());
        myNode = new ThreadLocal<QNode *>(n);
        for (int i = 0; i < n; i++)
        {
            myNode->arr[i]=new QNode();
        }   
        myPred = new ThreadLocal<QNode *>(nullptr, n);
    }
    void lock(int t_id)
    {
        QNode *qnode = myNode->get(t_id);
        qnode->locked = true;
        QNode *pred = tail.exchange(qnode);
        myPred->set(t_id, pred);
        while (pred->locked){}
    }
    void unlock(int t_id)
    {
        QNode *qnode = myNode->get(t_id);
        qnode->locked = false;
        myNode->set(t_id, myPred->get(t_id));
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
    test = new CLHLock(n);
    call_threads(n,"CLH");
    return 0; 
}