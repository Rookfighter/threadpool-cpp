#include <iostream>
#include <threadpool.h>

int main()
{
    // Start a new thread pool. This automatically starts its worker threads.
    // Specify the amount of threads in the constructor.
    // Omit the parameter to let the thread pool automatically detect a
    // suitable number.
    tpcpp::ThreadPool pool(4);

    std::cout << "Using " << pool.threads() << " threads" << std::endl;

    std::vector<tpcpp::Work::Ptr> workList(25);
    // run some work items
    for(size_t i = 0; i < workList.size(); ++i)
    {
        // the run() method expects a functor with no return value and parameters
        // it returns the created work item
        // the work item can be used to track the execution state, like Completed, Errored, etc.
        workList[i] = pool.run([i](){std::cout << "I got number " << i << std::endl;});
    }

    // wait for all tasks in the list to finish
    tpcpp::waitAll(workList);

    // create some data to operate on
    std::vector<double> data = {1, 2, 3, 4, 5, 6, 7, 8};

    // create a function that operates on the data and changes it in-place
    auto func = [](double &val){val *= val;};

    // execute the function on each element in parallel
    // foreach waits until the created tasks have finished
    pool.foreach(func, data);

    for(size_t i = 0; i < data.size(); ++i)
        std::cout << data[i] << ", ";

    std::cout << std::endl;

    return 0;
}