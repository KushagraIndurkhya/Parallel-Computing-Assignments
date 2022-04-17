#include <stdlib.h>
/* A QNode is a node in a queue */
class QNode
{
public:
    QNode *next;
    QNode *prev;
    bool locked;
/**
 * A constructor for the QNode class. It initializes the next and prev pointers to NULL and the locked
 * variable to false.
 */
    QNode()
    {
        next = NULL;
        prev = NULL;
        locked = false;
    }
};