//---------------------------------------------------------------------------------------------
/* This code is for implementing ThreadLocal functionality for class members. */
template <typename T>
class ThreadLocal
{
public:
    /**
     * It creates a new thread local variable.
     */
    ThreadLocal() {}
    T *arr;
    /**
     * `ThreadLocal` is a template class that takes a type `T` as a template parameter. It has a
     * constructor that takes an integer `n` as a parameter and creates an array of type `T` of size `n`
     *
     * @param n The number of elements in the array.
     */
    ThreadLocal(int n)
    {
        arr = new T[n];
    }
    /**
     * The function takes a value and an integer as arguments and creates an array of the given size and
     * initializes it with the given value
     *
     * @param val The value to be assigned to each element of the array.
     * @param n The number of threads that will be accessing the ThreadLocal object.
     */
    ThreadLocal(T val, int n)
    {
        arr = new T[n];
        for (int i = 0; i < n; i++)
            arr[i] = val;
    }
    /**
     * Set the value of the element at index id to val.
     *
     * @param id The id of the element you want to access. This is a 0-indexed number.
     * @param val The value to set the element to.
     */
    void set(int id, T val)
    {
        arr[id] = val;
    }
    /**
     * Returns the value of the element at the given index.
     *
     * @param id The id of the object you want to get.
     *
     * @return The value of the array at the index of the id.
     */
    T get(int id)
    {
        return arr[id];
    }
};